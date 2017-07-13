#include "../catch.hpp"
#include "../mocks/GameStateMock.h"
#include "../../Engine/StateManager.h"
#include "../../Engine/Kernel.h"

TEST_CASE("Map Loading functional tests", "[state][rsc_map][kernel]"){
	Kernel::Inst();

	SECTION( "Can Push to and Pop from StateManager") {
		auto stateSize = K_StateMan.stackSize();
		REQUIRE(stateSize == 0);

		K_StateMan.PushState(std::move(make_unique<GameStateMock>(&K_StateMan)));
		stateSize = K_StateMan.stackSize();
		REQUIRE(stateSize == 1);

		K_StateMan.PopState();
		stateSize = K_StateMan.stackSize();
		REQUIRE(stateSize == 0);

		stateSize = K_StateMan.stackSize();
		REQUIRE(stateSize == 0);
		REQUIRE(K_StateMan.IsEmpty());
	}

	SECTION( "State Update and Draw Called Correctly" ) {
		auto stateSmart = make_unique<GameStateMock>(&K_StateMan);
		auto state = stateSmart.get();
		K_StateMan.PushState(std::move(stateSmart));

		REQUIRE(state->init == true);
		REQUIRE(state->updateCount == 0);
		REQUIRE(state->drawCount == 0);
		REQUIRE(state->close == false);

		Kernel::Run();

		REQUIRE(state->init == true);
		REQUIRE(state->updateCount == 1);
		REQUIRE(state->drawCount == 1);
		REQUIRE(state->close == false);
	}

	SECTION( "Load Map from File"){
		//Assuming that the map has a width, height, and at least one entity
		REQUIRE_NOTHROW([&](){
			auto stateSmart = make_unique<GameStateMock>(&K_StateMan);
			auto state = stateSmart.get();
			K_StateMan.PushState(std::move(stateSmart));

			std::string mapName = "MAP1.tmx";
			auto mapToLoad = RSC_MapImpl::LoadResource(mapName);

			REQUIRE(mapToLoad.get() != NULL);
			REQUIRE(mapToLoad->GetWidthPixels() > 0 );
			REQUIRE(mapToLoad->GetHeightPixels() > 0 );

			state->SetCurrentMap(mapToLoad.get(), 0);
			REQUIRE(state->GetEntityMan()->GetEntityCount() > 0);
		}
		());
	}

	Kernel::Close();
}
