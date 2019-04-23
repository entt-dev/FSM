#pragma once

#include <entt/entt.hpp>
#include <iostream>
#include <chrono>
#include <taskflow/taskflow.hpp>


using Registry = entt::registry;
using EntityType = entt::entity;


/*----------  Subsection comment block  ----------*/

namespace Color {
  #define STATES \
      etype(Red) \
      etype(Green) \
      etype(Blue)

  #include "state_type_macro_snippet.h"
  #undef STATES

}  // namespace Color

namespace Movement {
  #define STATES \
      etype(Left) \
      etype(Jiggling) \
      etype(Rotating)

  #include "state_type_macro_snippet.h"
  #undef STATES

}  // namespace Status

namespace Status {
  #define STATES \
      etype(Stationary) \
      etype(Moving) \
      etype(Dead)

  #include "state_type_macro_snippet.h"
  #undef STATES

}  // namespace Status

struct Simulation {
  uint step{0};
  uint randomSeed{100};
  uint initialSize{100};
  uint preferredSize{200};
  bool parallelTests{false};
  bool parallelStates{true};
  bool doParallelAgents{false};
  bool printPerformance{false};
  bool printTotalPerformance{false};
  bool printTestPerformance{false};
};

class StopWatch {
  std::chrono::high_resolution_clock::time_point _start;
  std::chrono::duration<double> _duration;
  bool _perform{false};
public:
  StopWatch(bool perform = false) : _perform(perform) {}
  void start();
  void stepAndPrint(const std::string name);
};

class Fsm {
  tf::Taskflow _taskflow;

 public:
  Fsm() = default;
  void step(Registry& reg);
  void init(Registry& reg);
};

void makeTaskflow(Registry& reg, tf::Taskflow& tf);
bool canTransition(const Registry& reg, EntityType ent, float prob);
void step(Registry& reg);
void init(Registry& reg);
entt::prototype getAgentPrototype(Registry& reg);
entt::prototype getColoredAgentPrototype(Registry& reg);
