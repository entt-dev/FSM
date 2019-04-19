#include "./agents.h"



/*----------  Subsection comment block  ----------*/
namespace {

bool canTransition(float prob) {
  int whole = int(1.0 / prob);
  return rand() % whole == 0;
}

// in an actual real-world implementation, you'd not be templating out these
// transitions, but we can in our case since our tests to transition are all
// similar...
template<typename Alt1, typename Alt2>
void testColor(Registry& reg, EntityType ent) {
  uint step = reg.ctx<Simulation>().step;
  auto& entityState = reg.get<Color::EntityState>(ent);
  if (entityState.stepsSinceUpdated(step) < 10) return;
  if (canTransition(0.05) && entityState.canRevert()) {
    entityState.revertToPreviousState(step);
  } else if (canTransition(0.05)) {
    entityState.setNextState(Color::TypeToEnum<Alt1>::value, step);
  } else if (canTransition(0.05)) {
    entityState.setNextState(Color::TypeToEnum<Alt2>::value, step);
  }
}


// in an actual real-world implementation, you'd not be templating out these
// transitions, but we can in our case since our tests to transition are all
// similar...
template<typename Alt1, typename Alt2>
void testMovingTransition(Registry& reg, EntityType ent) {
  uint step = reg.ctx<Simulation>().step;
  auto& entityState = reg.get<Movement::EntityState>(ent);
  if (entityState.stepsSinceUpdated(step) < 10) return;
  if (canTransition(0.05) && entityState.canRevert()) {
    entityState.revertToPreviousState(step);
  } else if (canTransition(0.05)) {
    entityState.setNextState(Movement::TypeToEnum<Alt1>::value, step);
  } else if (canTransition(0.05)) {
    entityState.setNextState(Movement::TypeToEnum<Alt2>::value, step);
  }
}

template<typename Alt1>
void testStatus(Registry& reg, EntityType ent) {
  uint step = reg.ctx<Simulation>().step;
  auto& entityState = reg.get<Status::EntityState>(ent);
  if (entityState.stepsSinceUpdated(step) < 10) return;
  if (canTransition(0.05) && entityState.canRevert()) {
    entityState.revertToPreviousState(step);
  } else if (canTransition(0.05)) {
    entityState.setNextState(Status::TypeToEnum<Alt1>::value, step);
  }
}


void testDead(Registry& reg, EntityType ent) {
  uint step = reg.ctx<Simulation>().step;
  if (step < 100) return; // no dying till 100 steps at least.
  auto& entityState = reg.get<Status::EntityState>(ent);
  if (canTransition(0.01)) {
    entityState.setNextState(Status::ST_Dead, step);
  }
}

} // anonymous namespace


void updatePerAgent(Registry& reg) {
  auto group = reg.group<Status::EntityState, Movement::EntityState>();

  #pragma omp parallel for
  for (uint i = 0; i < group.size(); ++i) {
    EntityType ent = group.data()[i];
    {
      const auto& state = reg.get<Movement::EntityState>(ent);
      switch (state.current()) {
        case Movement::ST_Left: {
          testMovingTransition<Movement::Jiggling, Movement::Rotating>(reg, ent);
        } break;
        case Movement::ST_Jiggling: {
          testMovingTransition<Movement::Left, Movement::Rotating>(reg, ent);
        } break;
        case Movement::ST_Rotating: {
          testMovingTransition<Movement::Left, Movement::Left>(reg, ent);
        } break;
      }
    }

    {
      const auto& state = reg.get<Status::EntityState>(ent);
      switch (state.current()) {
        case Status::ST_Moving: {
          testStatus<Status::Stationary>(reg, ent);
        } break;
        case Status::ST_Stationary: {
          testStatus<Status::Moving>(reg, ent);
        } break;
      }
    }

    if (reg.has<Color::EntityState>(ent)) {
      const auto& state = reg.get<Color::EntityState>(ent);
      switch (state.current()) {
        case Color::ST_Red: {
          testColor<Color::Green, Color::Blue>(reg, ent);
        } break;
        case Color::ST_Green: {
          testColor<Color::Red, Color::Blue>(reg, ent);
        } break;
        case Color::ST_Blue: {
          testColor<Color::Green, Color::Red>(reg, ent);
        } break;
      }
    }
    testDead(reg, ent);
  }
}