#include "gtest/gtest.h"
#include "fsm.h"
#include "components.h"


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(Fsm, Group) {

  Registry reg;
  // this is invalid apparently...
  reg.group<Data::Position>(entt::get<Data::Orient>);
  // reg.group<Data::Orient>(entt::get<Data::Position>);
  reg.group<Data::Velocity>(entt::get<Data::Orient>);

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



TEST(Fsm, Random) {

  Registry reg;
  reg.set<Simulation>();
  auto& sim = reg.ctx<Simulation>();
  sim.randomSeed = rand();
  sim.randomSeed = rand();
  // sim.randomSeed = rand();

  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < 100; ++i) {
    agent.create();
    coloredAgent.create();
  }

  int numChanged{0};
  for (auto ent : reg.view<Data::Position>()) {
    if (canTransition(reg, ent, 0.5)) {
      ++numChanged;
    }
  }
  ASSERT_GE(numChanged, 0);
}


TEST(Fsm, Run) {

  Registry reg;


  init(reg);
  auto& sim = reg.ctx<Simulation>();
  sim.preferredSize = 600;
  sim.printPerformance = true;
  sim.printTotalPerformance = true;
  sim.printTestPerformance = true;

  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < 300; ++i) {
    agent.create();
    coloredAgent.create();
  }
  ASSERT_EQ(reg.size<Data::Position>(), 600);
  ASSERT_EQ(reg.size<Status::EntityState>(), 600);
  ASSERT_EQ(reg.size<Color::EntityState>(), 300);

  // auto debugEnt = reg.data<Color::EntityState>()[0];
  Fsm fsm;

  for (int i = 0; i < 200; ++i) {
    fsm.step(reg);
    // debugAgent(reg, debugEnt);
  }
}


TEST(Fsm, AgentsRun) {

  Registry reg;

  init(reg);
  auto& sim = reg.ctx<Simulation>();
  sim.preferredSize = 600;
  sim.printPerformance = true;
  sim.printTotalPerformance = true;
  sim.printTestPerformance = true;
  sim.parallelTests = false;
  sim.parallelStates = false;
  sim.doParallelAgents = true;

  auto agent = getAgentPrototype(reg);
  auto coloredAgent = getColoredAgentPrototype(reg);

  for (int i = 0; i < 300; ++i) {
    agent.create();
    coloredAgent.create();
  }
  ASSERT_EQ(reg.size<Data::Position>(), 600);
  ASSERT_EQ(reg.size<Status::EntityState>(), 600);
  ASSERT_EQ(reg.size<Color::EntityState>(), 300);

  // auto debugEnt = reg.data<Color::EntityState>()[0];
  Fsm fsm;

  for (int i = 0; i < 200; ++i) {
    fsm.step(reg);
    // debugAgent(reg, debugEnt);
  }
}
