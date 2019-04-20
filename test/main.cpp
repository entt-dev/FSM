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

  for (int i = 0; i < 100; ++i) {
    step(reg);
  }


  // reg.view<const Status::EntityState>().each([](const auto& st) {
  //   std::cout << "color " << enum2str(st.current()) << std::endl;
  // });
  // reg.view<const Color::EntityState>().each([](const auto& st) {
  //   std::cout << "color " << enum2str(st.current()) << std::endl;
  // });
  // reg.view<const Movement::EntityState>().each([](const auto& st) {
  //   std::cout << "color " << enum2str(st.current()) << std::endl;
  // });
}
