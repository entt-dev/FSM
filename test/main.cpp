#include "gtest/gtest.h"
#include "fsm.h"
#include "components.h"


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

template<typename T>
void debugState(const Registry& reg, EntityType id, const char* name) {
  uint step = reg.ctx<const Simulation>().step;

  const auto& st = reg.get<const T>(id);
  if (!st.isChangedThisStep(step)) return;
  std::cout << name << " : " << step << " : "
    << enum2str(st.previous()) << "->"
    << enum2str(st.current()) << std::endl;
}

void debugAgent(const Registry& reg, EntityType id) {
  if (!reg.valid(id)) return;
   debugState<Color::EntityState>(reg, id, "Color");
   debugState<Movement::EntityState>(reg, id, "Color");
   debugState<Status::EntityState>(reg, id, "Color");
}


TEST(Fsm, Run) {

  Registry reg;


  init(reg);

  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < 100; ++i) {
    agent.create();
    coloredAgent.create();
  }
  ASSERT_EQ(reg.size<Data::Position>(), 200);
  ASSERT_EQ(reg.size<Status::EntityState>(), 200);
  ASSERT_EQ(reg.size<Color::EntityState>(), 100);

  auto debugEnt = reg.data<Color::EntityState>()[0];

  for (int i = 0; i < 200; ++i) {
    step(reg);
    debugAgent(reg, debugEnt);
  }
}
