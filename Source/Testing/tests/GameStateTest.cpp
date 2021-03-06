#include "../../Engine/Kernel.h"
#include "../../Engine/StateManager.h"
#include "../catch.hpp"
#include "../mocks/GameStateMock.h"

TEST_CASE("Map Loading functional tests", "[state][rsc_map][kernel]") {
  Kernel::Inst();

  SECTION("Can Push to and Pop from StateManager") {
    auto stateSize = K_StateMan.StackSize();
    REQUIRE(stateSize == 1);

    K_StateMan.PushState(
        std::move(std::make_unique<GameStateMock>(&K_StateMan)));
    // Update stateman, causing the new state to be pushed
    Kernel::Update();
    stateSize = K_StateMan.StackSize();
    REQUIRE(stateSize == 2);

    K_StateMan.PopState();
    // Update stateman, causing the new state to be popped
    Kernel::Update();
    stateSize = K_StateMan.StackSize();
    REQUIRE(stateSize == 1);

    K_StateMan.PopState();
    Kernel::Update();
    stateSize = K_StateMan.StackSize();
    REQUIRE(stateSize == 0);
    REQUIRE(K_StateMan.IsEmpty());
  }

  SECTION("State Update and Draw Called Correctly") {
    auto stateSmart = std::make_unique<GameStateMock>(&K_StateMan);
    auto state = stateSmart.get();
    K_StateMan.PushState(std::move(stateSmart));

    REQUIRE(state->init == false);
    REQUIRE(state->updateCount == 0);
    REQUIRE(state->drawCount == 0);
    REQUIRE(state->close == false);

    Kernel::Update();

    REQUIRE(state->init == true);
    REQUIRE(state->updateCount == 1);
    REQUIRE(state->drawCount == 1);
    REQUIRE(state->close == false);
  }

  //Need a test map for this section
  /*
  SECTION("Load Map from File") {
    // Assuming that the map has a width, height, and at least one entity
    REQUIRE_NOTHROW([&]() {
      auto stateSmart = std::make_unique<GameStateMock>(&K_StateMan);
      auto state = stateSmart.get();
      K_StateMan.PushState(std::move(stateSmart));

      Kernel::Update();

      std::string mapName = "MAP1.tmx";
      auto mapToLoad = RSC_MapImpl::LoadResource(mapName);

      REQUIRE(mapToLoad.get() != NULL);
      REQUIRE(mapToLoad->GetWidthPixels() > 0);
      REQUIRE(mapToLoad->GetHeightPixels() > 0);

      state->SetMapNextFrame(mapToLoad.get(), 0);
      Kernel::Update();
      REQUIRE(state->GetEntityMan()->GetEntityCount() > 0);
    }());
  }
  */

  Kernel::Close();
}
