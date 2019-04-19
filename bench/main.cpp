#include <benchmark/benchmark.h>
#include "fsm.h"

void stepSimulation(benchmark::State& state) {

  Registry reg;


  init(reg);

  int numAgents = 10000;

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


BENCHMARK(stepSimulation)->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();
