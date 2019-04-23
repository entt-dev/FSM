
#include "./fsm.h"
#include "./components.h"
#include "./agents.h"
#include <algorithm>
#include <parallel/algorithm>


/*----------  Subsection comment block  ----------*/

bool canTransition(const Registry& reg, EntityType ent, float prob) {
  int whole = int(1.0 / prob);
  // get a number from 0, 100 if prob == 0.01...
  // we don't want rand() being called in multiple threads...
  // whole = reg.ctx<Simulation>().randomSeed() % whole;
  auto origEnt = ent;
  ent += (reg.ctx<Simulation>().randomSeed);
  ent = ent % reg.size();
  ent = std::max(ent, EntityType(1));
  assert(whole > 0);
  // std::cout << "Ent ? " << ent << " " << whole << " " << prob << std::endl;
  // update certain percentage of agents...
  // if (ent % whole == 0) std::cout << "ent ? " << origEnt << " " << ent << " " << whole << " " << reg.ctx<Simulation>().randomSeed << std::endl;
  if (ent % whole == 0) return true;
  return false;
  // return rand() % whole == 0;
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
  bool printTestPerformance = reg.ctx<Simulation>().printTestPerformance;
  StopWatch watch(printTestPerformance);
  watch.start();
  // for (auto id : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType id) {

  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto ent = group.data()[i];
    auto& entityState = group.template get<Color::EntityState>(ent);
    assert(entityState.current() == Color::TypeToEnum<Current>::value);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(reg, ent, 0.05) && entityState.canRevert()) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(reg, ent, 0.05)) {
      entityState.setNextState(Color::TypeToEnum<Alt1>::value, step);
    } else if (canTransition(reg, ent, 0.05)) {
      entityState.setNextState(Color::TypeToEnum<Alt2>::value, step);
    }
  }
  std::string s = enum2str(Color::TypeToEnum<Current>::value);
  std::string text = "ColorTest [us] " + s + " : " + std::to_string(group.size()) + " : ";
  watch.stepAndPrint(text);
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
  bool printTestPerformance = reg.ctx<Simulation>().printTestPerformance;
  StopWatch watch(printTestPerformance);
  watch.start();
  // for (auto ent : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType ent) {

  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto ent = group.data()[i];
    auto& entityState = group.template get<Movement::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(reg, ent, 0.05) && entityState.canRevert()) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(reg, ent, 0.05)) {
      entityState.setNextState(Movement::TypeToEnum<Alt1>::value, step);
    } else if (canTransition(reg, ent, 0.05)) {
      entityState.setNextState(Movement::TypeToEnum<Alt2>::value, step);
    }
  }
  std::string s = enum2str(Movement::TypeToEnum<Current>::value);
  std::string text = "MovingTest [us] " + s + " : " + std::to_string(group.size()) + " : ";
  watch.stepAndPrint(text);
}

template<typename Current, typename Alt1>
void testStatus(Registry& reg) {
  auto group = reg.group<Current>(entt::get<Status::EntityState>);

  static_assert(Status::TypeToEnum<Status::Moving>::value == Status::ST_Moving);
  static_assert(Status::TypeToEnum<Status::Stationary>::value == Status::ST_Stationary);

  uint step = reg.ctx<Simulation>().step;
  bool printTestPerformance = reg.ctx<Simulation>().printTestPerformance;
  StopWatch watch(printTestPerformance);
  watch.start();
  // for (auto ent : group) {
  // __gnu_parallel::for_each(group.begin(), group.end(),
  //     [step, &group](EntityType ent) {
  #pragma omp parallel for if (reg.ctx<Simulation>().parallelTests)
  for (uint i = 0; i < group.size(); ++i) {
    auto ent = group.data()[i];
    auto& entityState = group.template get<Status::EntityState>(ent);
    if (entityState.stepsSinceUpdated(step) < 10) continue;
    if (canTransition(reg, ent, 0.05) && entityState.canRevert()) {
      entityState.revertToPreviousState(step);
    } else if (canTransition(reg, ent, 0.05)) {
      entityState.setNextState(Status::TypeToEnum<Alt1>::value, step);
    }
  }
  std::string s = enum2str(Status::TypeToEnum<Current>::value);
  std::string text = "StatusTest [us] " + s + " : " + std::to_string(group.size()) + " : ";
  watch.stepAndPrint(text);
}

void testDead(Registry& reg) {

  auto group = reg.group<>(entt::get<Status::EntityState>, entt::exclude<Status::Dead>);

  uint step = reg.ctx<Simulation>().step;
  if (step < 100) return; // no dying till 100 steps at least.
  for (auto ent : group) {
    auto& entityState = group.get<Status::EntityState>(ent);
    if (canTransition(reg, ent, 0.01)) {
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

// template<typename T>
// void makeTasks(tf::Taskflow tf, Registry& reg, T t) {
//   tf.emplace([&] { t(reg); });
// }



void makeTaskflow(Registry& reg, tf::Taskflow& tf) {
  auto items = tf.emplace(
    [&reg]() {
      testMovingTransitions<Movement::Left, Movement::Jiggling, Movement::Rotating>(reg);
    },
    [&reg]() {
      testMovingTransitions<Movement::Jiggling, Movement::Left, Movement::Rotating>(reg);
    },
    [&reg]() {
      testMovingTransitions<Movement::Rotating, Movement::Left, Movement::Left>(reg);
    },
    [&reg]() {
      testColor<Color::Red, Color::Green, Color::Blue>(reg);
    },
    [&reg]() {
      testColor<Color::Green, Color::Red, Color::Blue>(reg);
    },
    [&reg]() {
      testColor<Color::Blue, Color::Green, Color::Red>(reg);
    },
    [&reg]() {
      testStatus<Status::Moving, Status::Stationary>(reg);
    },
    [&reg]() {
      testStatus<Status::Stationary, Status::Moving>(reg);
    }
  );
}

void updateStates(Registry& reg, tf::Taskflow& tf, bool parallel) {

  // uint numWorkers = std::thread::hardware_concurrency();
  if (!parallel) {
    tf::Taskflow tserial{0};
    makeTaskflow(reg, tserial);
    tserial.wait_for_all();
  } else {
    tf.wait_for_all();
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
uint updateTags(Registry& reg) {
  uint step = reg.ctx<Simulation>().step;
  auto group = reg.group<StateEnum>(entt::exclude<Status::Dead>);

  uint numChanged{0};
  group.each([&reg, step, &numChanged](EntityType id, auto& state) {
    if (state.isChangedThisStep(step)) {
      assert(state.current() != state.previous());
      // remove all other states if they belong to the agent...
      // since the previous state may not necessarily match.
      // this is because the state could change multiple times by different
      // systems while in serial mode. e.g see the dead system.
      StateEnum::removeAllTags(reg, id);
      // now we assign the only tag allowed for this state.
      StateEnum::assignTagByEnum(reg, state.current(), id);
      ++numChanged;
    }
  });
  return numChanged;
}

void updateStateTags(Registry& reg, bool parallel) {

  // I get errors and conflicts when trying to update
  // multiple component tags at once due to group ownership likely...

  auto debug = reg.ctx<Simulation>().printPerformance;
  uint numChanged{0};
  numChanged = updateTags<Status::EntityState>(reg);
  if (debug) std::cout << " stat changed " << numChanged;
  numChanged = updateTags<Color::EntityState>(reg);
  if (debug) std::cout << " colo " << numChanged;
  numChanged = updateTags<Movement::EntityState>(reg);
  if (debug) std::cout << " move " << numChanged << std::endl;

}

uint spawnAgents(Registry& reg) {
  const auto& s = reg.ctx<Simulation>();
  auto currentSize = reg.size<Status::EntityState>();
  if (currentSize >= s.preferredSize) return 0;
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
  return numToSpawn;
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

// void step(Registry& reg) {
//   ++reg.ctx<Simulation>().step;
//   reg.ctx<Simulation>().randomSeed = rand();
//   spawnAgents(reg);
//   if (reg.ctx<Simulation>().doParallelAgents) {
//     updatePerAgent(reg);
//   } else {
//     updateStates(reg, reg.ctx<Simulation>().parallelStates);
//     updateStateTags(reg, false);
//   }
//   cleanupDead(reg);
// }
template <typename... Type, template <typename...> class T>
void reserveByList(const T<Type...>&, Registry& reg) {
  auto preferredSize = reg.ctx<Simulation>().preferredSize;
  (reg.reserve<Type>(preferredSize), ...);
}


void Fsm::init(Registry& reg) {
  reg.set<Simulation>();

  reserveByList(Color::EntityState::stateTypeList, reg);
  reserveByList(Status::EntityState::stateTypeList, reg);
  reserveByList(Movement::EntityState::stateTypeList, reg);

  makeTaskflow(reg, _taskflow);
  updateStates(reg, _taskflow, false);
}

void Fsm::step(Registry& reg) {
  ++reg.ctx<Simulation>().step;
  reg.ctx<Simulation>().randomSeed = rand();
  bool printPerformance = reg.ctx<Simulation>().printPerformance;
  bool printTotalPerformance = reg.ctx<Simulation>().printTotalPerformance;
  StopWatch watch(printPerformance);
  StopWatch total(printTotalPerformance);
  total.start();
  auto numSpawned = spawnAgents(reg);
  if (printPerformance) std::cout << " num spawned " << numSpawned << std::endl;
  if (reg.ctx<Simulation>().doParallelAgents) {
    updatePerAgent(reg);
  } else {
    watch.start();
    updateStates(reg, _taskflow, reg.ctx<Simulation>().parallelStates);
    watch.stepAndPrint("updateStates us: ");
    updateStateTags(reg, false);
    watch.stepAndPrint("updateStateTags us: ");
  }
  cleanupDead(reg);
  total.stepAndPrint("total us: ");
  if (printTotalPerformance) std::cout << "~~~~~~~" << std::endl;
}

void StopWatch::start() {
  if (!_perform) return;
  _start = std::chrono::high_resolution_clock::now();
}

void StopWatch::stepAndPrint(const std::string name) {
  if (!_perform) return;
  typedef std::chrono::microseconds us;
  auto tmp = std::chrono::high_resolution_clock::now();
  _duration = (tmp - _start);
  _start = tmp;

  std::string text = name;
  text += std::to_string(std::chrono::duration_cast<us>(_duration).count());
  text += "\n";

  std::cout <<  text;
}


// void init(Registry& reg) {
//   reg.set<Simulation>();

//   reserveByList(Color::EntityState::stateTypeList, reg);
//   reserveByList(Status::EntityState::stateTypeList, reg);
//   reserveByList(Movement::EntityState::stateTypeList, reg);

// }
