#include <benchmark/benchmark.h>
#include "fsm.h"



bool canTransition(float prob) {
  int whole = int(1.0 / prob);
  return rand() % whole == 0;
}

void testRandom(benchmark::State& state) {
  while (state.KeepRunning()) {
    canTransition(0.5);
  }
}

void benchTest(benchmark::State& state) {

  Registry reg;


  init(reg);

  int numAgents = 1000 / 2;

  reg.ctx<Simulation>().preferredSize = numAgents;
  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < numAgents; ++i) {
    agent.create();
    coloredAgent.create();
  }

  // std::cout << "SIZE " << reg.size<Color::Red>() << std::endl;
  assert(reg.size<Color::Red>() == numAgents);

  while (state.KeepRunning()) {
    auto group = reg.group<Color::Red>(entt::get<Color::EntityState>, entt::exclude<Status::Dead>);
    int step = 100;
    for (uint i = 0; i < group.size(); ++i) {
      auto id = group.data()[i];
      auto& entityState = group.template get<Color::EntityState>(id);
      if (entityState.stepsSinceUpdated(step) < 10) continue;
      if (canTransition(0.05) && entityState.canRevert()) {
        entityState.revertToPreviousState(step);
      } else if (canTransition(0.05)) {
        entityState.setNextState(Color::TypeToEnum<Color::Green>::value, step);
      } else if (canTransition(0.05)) {
        entityState.setNextState(Color::TypeToEnum<Color::Blue>::value, step);
      }
    }
  }

}

void stepSimulationParallelStates(benchmark::State& state) {

  Registry reg;


  init(reg);

  const int numAgents = 10000 / 2;

  reg.ctx<Simulation>().preferredSize = numAgents;
  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < numAgents; ++i) {
    agent.create();
    coloredAgent.create();
  }

  while (state.KeepRunning()) {
    step(reg);
  }

}


void stepSimulationParallelTests(benchmark::State& state) {

  Registry reg;


  init(reg);

  const int numAgents = 10000 / 2;

  auto& sim = reg.ctx<Simulation>();
  sim.preferredSize = numAgents;
  sim.parallelTests = true;
  sim.parallelStates = false;
  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < numAgents; ++i) {
    agent.create();
    coloredAgent.create();
  }

  while (state.KeepRunning()) {
    step(reg);
  }

}

void stepSimulationParallelAgents(benchmark::State& state) {

  Registry reg;
  init(reg);
  const int numAgents = 10000 / 2;

  auto& sim = reg.ctx<Simulation>();
  sim.preferredSize = numAgents;
  sim.parallelTests = false;
  sim.parallelStates = false;
  sim.doParallelAgents = true;
  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < numAgents; ++i) {
    agent.create();
    coloredAgent.create();
  }

  while (state.KeepRunning()) {
    step(reg);
  }

}


BENCHMARK(stepSimulationParallelAgents)->Unit(benchmark::kMillisecond);
BENCHMARK(stepSimulationParallelTests)->Unit(benchmark::kMillisecond);
BENCHMARK(stepSimulationParallelStates)->Unit(benchmark::kMillisecond);
BENCHMARK(testRandom)->Unit(benchmark::kNanosecond);
BENCHMARK(benchTest)->Unit(benchmark::kMicrosecond);


BENCHMARK_MAIN();
