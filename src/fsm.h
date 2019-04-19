#pragma once

#include <entt/entt.hpp>
#include <iostream>


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
  uint initialSize{100};
  uint preferredSize{200};
  bool parallelTests{false};
  bool parallelStates{true};
  bool doParallelAgents{false};
};

void step(Registry& reg);
void init(Registry& reg);
entt::prototype getAgentPrototype(Registry& reg);
entt::prototype getColoredAgentPrototype(Registry& reg);
