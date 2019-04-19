
#include "./fsm.h"
#include "./components.h"



/*----------  Subsection comment block  ----------*/

bool canTransition(float prob) {
  int whole = int(1.0 / prob);
  return rand() % whole == 0;
}

// in an actual real-world implementation, you'd not be templating out these
// transitions, but we can in our case since our tests to transition are all
// similar...
template<typename Current, typename Alt1, typename Alt2>
void testColor(Registry& reg) {
  auto group = reg.group<Current>(entt::get<Color::EntityState>);

  static_assert(Color::TypeToEnum<Color::Red>::value == Color::ST_Red);
  static_assert(Color::TypeToEnum<Color::Green>::value == Color::ST_Green);
  static_assert(Color::TypeToEnum<Color::Blue>::value == Color::ST_Blue);

  uint step = reg.ctx<Simulation>().step;
  group.each([step](auto currentColor, auto& entityState) {
    if (entityState.stepsSinceUpdated(step) < 10) return;
    if (canTransition(0.05)) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(0.05)) {
      Color::StateType v = Color::TypeToEnum<Alt1>::value;
      entityState.setNextState(Color::TypeToEnum<Alt1>::value, step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Color::TypeToEnum<Alt2>::value, step);
    }
  });
}


// in an actual real-world implementation, you'd not be templating out these
// transitions, but we can in our case since our tests to transition are all
// similar...
template<typename Current, typename Alt1, typename Alt2>
void testMovingTransitions(Registry& reg) {
  auto group = reg.group<Current>(entt::get<Status::Moving, Movement::EntityState>);

  static_assert(Movement::TypeToEnum<Movement::Left>::value == Movement::ST_Left);
  static_assert(Movement::TypeToEnum<Movement::Jiggling>::value == Movement::ST_Jiggling);
  static_assert(Movement::TypeToEnum<Movement::Rotating>::value == Movement::ST_Rotating);

  uint step = reg.ctx<Simulation>().step;
  for (auto ent : group) {
    auto& entityState = group.template get<Movement::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(0.05)) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Movement::TypeToEnum<Alt1>::value, step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Movement::TypeToEnum<Alt2>::value, step);
    }
  }
}

template<typename Current, typename Alt1>
void testStatus(Registry& reg) {
  auto group = reg.group<Current>(entt::get<Status::EntityState>);

  static_assert(Status::TypeToEnum<Status::Moving>::value == Status::ST_Moving);
  static_assert(Status::TypeToEnum<Status::Stationary>::value == Status::ST_Stationary);

  uint step = reg.ctx<Simulation>().step;
  for (auto ent : group) {
    auto& entityState = group.template get<Status::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(0.05)) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Status::TypeToEnum<Alt1>::value, step);
    }
  }
}

void testDead(Registry& reg) {

  auto group = reg.group<>(entt::get<Status::EntityState>, entt::exclude<Status::Dead>);

  uint step = reg.ctx<Simulation>().step;
  if (step < 100) return; // no dying till 100 steps at least.
  for (auto ent : group) {
    auto& entityState = group.get<Status::EntityState>(ent);
    if (canTransition(0.01)) {
      entityState.setNextState(Status::ST_Dead, step);
    }
  }
}

entt::prototype getCommonPrototype(Registry& registry) {
  entt::prototype proto{registry};
  proto.set<Data::Position>();
  proto.set<Data::Velocity>();
  proto.set<Data::Orient>();
  return proto;
}

entt::prototype getAgentPrototype(Registry& registry) {
  auto proto = getCommonPrototype(registry);

  Movement::setDefaultState<Movement::Left>(proto);
  Status::setDefaultState<Status::Stationary>(proto);
  return proto;
}

entt::prototype getColoredAgentPrototype(Registry& registry) {
  auto proto = getCommonPrototype(registry);

  Color::setDefaultState<Color::Red>(proto);
  Movement::setDefaultState<Movement::Left>(proto);
  Status::setDefaultState<Status::Stationary>(proto);
  return proto;
}

void updateStates(Registry& reg) {
  #pragma omp parallel sections
  {
    #pragma omp section
    {
      testMovingTransitions<Movement::Left, Movement::Jiggling, Movement::Rotating>(reg);
    }
    #pragma omp section
    {
      testMovingTransitions<Movement::Jiggling, Movement::Left, Movement::Rotating>(reg);
    }
    #pragma omp section
    {
      testMovingTransitions<Movement::Rotating, Movement::Left, Movement::Left>(reg);
    }
    #pragma omp section
    {
      testColor<Color::Red, Color::Green, Color::Blue>(reg);
    }
    #pragma omp section
    {
      testColor<Color::Green, Color::Red, Color::Blue>(reg);
    }
    #pragma omp section
    {
      testColor<Color::Blue, Color::Green, Color::Red>(reg);
    }
    #pragma omp section
    {
      testStatus<Status::Moving, Status::Stationary>(reg);
    }
    #pragma omp section
    {
      testStatus<Status::Stationary, Status::Moving>(reg);
    }
  }

  // since test dead checks all states, it's not thread safe...
  testDead(reg);
}

template<class StateEnum>
void updateTags(Registry& reg) {
  uint step = reg.ctx<Simulation>().step;
  auto group = reg.group<StateEnum>(entt::exclude<Status::Dead>);
  group.each([&reg, step](EntityType id, auto& state) {
    if (state.isChangedThisStep(step)) {
      std::cout << "REMOVE " << state << std::endl;
      StateEnum::removeTagByEnum(reg, state.previous(), id);
      StateEnum::assignTagByEnum(reg, state.current(), id);
      std::cout << "DONE " << state << std::endl;
    }
  });
}

void updateStateTags(Registry& reg) {

  updateTags<Status::EntityState>(reg);
  updateTags<Color::EntityState>(reg);
  updateTags<Movement::EntityState>(reg);

}

void step(Registry& reg) {
  ++reg.ctx<Simulation>().step;
  updateStates(reg);
  updateStateTags(reg);
}

void init(Registry& reg) {
  reg.set<Simulation>();

}
