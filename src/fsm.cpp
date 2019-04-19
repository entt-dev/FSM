
#include "./fsm.h"
#include "./components.h"
#include "./agents.h"
#include <parallel/algorithm>


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
  auto group = reg.group<Current>(entt::get<Color::EntityState>, entt::exclude<Status::Dead>);

  static_assert(Color::TypeToEnum<Color::Red>::value == Color::ST_Red);
  static_assert(Color::TypeToEnum<Color::Green>::value == Color::ST_Green);
  static_assert(Color::TypeToEnum<Color::Blue>::value == Color::ST_Blue);

  uint step = reg.ctx<Simulation>().step;
  // for (auto id : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType id) {

  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto id = group.data()[i];
    auto& entityState = group.template get<Color::EntityState>(id);
    assert(entityState.current() == Color::TypeToEnum<Current>::value);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(0.05) && entityState.canRevert()) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Color::TypeToEnum<Alt1>::value, step);
    } else if (canTransition(0.05)) {
      entityState.setNextState(Color::TypeToEnum<Alt2>::value, step);
    }
  }
}


// in an actual real-world implementation, you'd not be templating out these
// transitions, but we can in our case since our tests to transition are all
// similar...
template<typename Current, typename Alt1, typename Alt2>
void testMovingTransitions(Registry& reg) {
  auto group = reg.group<Current>(
    entt::get<Status::Moving, Movement::EntityState>, entt::exclude<Status::Dead>
  );

  static_assert(Movement::TypeToEnum<Movement::Left>::value == Movement::ST_Left);
  static_assert(Movement::TypeToEnum<Movement::Jiggling>::value == Movement::ST_Jiggling);
  static_assert(Movement::TypeToEnum<Movement::Rotating>::value == Movement::ST_Rotating);

  uint step = reg.ctx<Simulation>().step;
  // for (auto ent : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType ent) {

  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto ent = group.data()[i];
    auto& entityState = group.template get<Movement::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(0.05) && entityState.canRevert()) {
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
  // for (auto ent : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType ent) {
  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto ent = group.data()[i];
    auto& entityState = group.template get<Status::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(0.05) && entityState.canRevert()) {
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

void updateStates(Registry& reg, bool parallel) {

  #pragma omp parallel sections if (parallel)
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
  // if doesn't really matter if we testdead at start or end,
  // since the Dead tag doesn't happen till all states have been updated,
  // so the filtering won't happen till next state.
  testDead(reg);
}


// template <typename EntSt, typename... Type, template <typename...> class T>
// void resetAllStatesOfType(const T<Type...>&, Registry& reg) {
//   // (reg.reset<Type>(), ...);
//   (EntSt::removeTagByEnum)
// }

template<class StateEnum>
void updateTags(Registry& reg) {
  uint step = reg.ctx<Simulation>().step;
  auto group = reg.group<StateEnum>(entt::exclude<Status::Dead>);

  group.each([&reg, step](EntityType id, auto& state) {
    if (state.isChangedThisStep(step)) {
      assert(state.current() != state.previous());
      // remove all other states if they belong to the agent...
      // since the previous state may not necessarily match.
      // this is because the state could change multiple times by different
      // systems while in serial mode. e.g see the dead system.
      StateEnum::removeAllTags(reg, id);
      // now we assign the only tag allowed for this state.
      StateEnum::assignTagByEnum(reg, state.current(), id);
    }
  });
}

void updateStateTags(Registry& reg, bool parallel) {

  // I get errors and conflicts when trying to update
  // multiple component tags at once due to group ownership likely...
  #pragma omp parallel sections if (false)
  {
    #pragma omp section
    {
        // std::cout << "SECTION 1 " << std::endl;
      updateTags<Status::EntityState>(reg);
        // std::cout << "SECTION 1 END " << std::endl;
    }
    #pragma omp section
    {
        // std::cout << "SECTION 2 " << std::endl;
      updateTags<Color::EntityState>(reg);
        // std::cout << "SECTION 2 END " << std::endl;
    }
    #pragma omp section
    {
        // std::cout << "SECTION 3 " << std::endl;
      updateTags<Movement::EntityState>(reg);
        // std::cout << "SECTION 3 END " << std::endl;
    }
  }

}

void spawnAgents(Registry& reg) {
  const auto& s = reg.ctx<Simulation>();
  auto currentSize = reg.size<Status::EntityState>();
  if (currentSize >= s.preferredSize) return;
  assert(currentSize < s.preferredSize);
  auto diff =  s.preferredSize - currentSize;
  uint numToSpawn = rand() % diff;

  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (uint i = 0; i < numToSpawn; ++i) {
    if (rand() % 2 == 0) {
      agent.create();
    } else {
      coloredAgent.create();
    }
  }
}

void cleanupDead(Registry& reg) {
  uint step = reg.ctx<Simulation>().step;
  reg.group<Status::Dead>(entt::get<Status::EntityState>).each([
    &reg,
    step
  ](
    auto id,
    const auto& dead, const auto& st) {
    if (st.stepsSinceUpdated(step) > 10) reg.destroy(id);
  });
}

void step(Registry& reg) {
  ++reg.ctx<Simulation>().step;
  spawnAgents(reg);
  if (reg.ctx<Simulation>().doParallelAgents) {
    updatePerAgent(reg);
  } else {
    updateStates(reg, reg.ctx<Simulation>().parallelStates);
    updateStateTags(reg, false);
  }
  cleanupDead(reg);
}

template <typename... Type, template <typename...> class T>
void reserveByList(const T<Type...>&, Registry& reg) {
  (reg.reserve<Type>(10), ...);
}


void init(Registry& reg) {
  reg.set<Simulation>();

  reserveByList(Color::EntityState::stateTypeList, reg);
  reserveByList(Status::EntityState::stateTypeList, reg);
  reserveByList(Movement::EntityState::stateTypeList, reg);


  // we need to call all the groups initially serially, since groups cannot be
  // initialized in parallel on the same registry...
  updateStates(reg, false);
  // updateStateTags(reg, false);
}
