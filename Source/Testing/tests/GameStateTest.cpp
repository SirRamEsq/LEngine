#include "../catch.hpp"

#include "../../Engine/StateManager.h"
#include "../../Engine/Kernel.h"

class _test_State : public GameState{
	public:
	_test_State(GameStateManager* gm)
		:GameState(gm){
		init=false;
		close=false;
		updateCount=0;
		drawCount=0;
	}
	void Init(){
		init = true;
	}
	void Close(){
		close = true;
	}
	void HandleEvent(const Event*){
	}
	bool Update(){
		updateCount+=1;
		return true;
	}
	void Draw(){
		drawCount +=1;
	}

	~_test_State(){

	}

	int drawCount;
	int updateCount;
	bool close;
	bool init;
};

TEST_CASE("Map Loading functional tests", "[state][rsc_map][kernel]"){
	Kernel::Inst();

	SECTION( "Can Push to and Pop from StateManager") {
		auto stateSize = K_StateMan.stackSize();
		REQUIRE(stateSize == 0);

		K_StateMan.PushState(std::move(make_unique<_test_State>(&K_StateMan)));
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
		auto stateSmart = make_unique<_test_State>(&K_StateMan);
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
		REQUIRE_NOTHROW([&](){
		auto stateSmart = make_unique<_test_State>(&K_StateMan);
		auto state = stateSmart.get();
		K_StateMan.PushState(std::move(stateSmart));

		std::string mapName = "MAP1.tmx";
		auto mapToLoad = RSC_MapImpl::LoadResource(mapName);

		REQUIRE(mapToLoad.get() != NULL);
		//REQUIRE(mapToLoad->GetWidthPixels() == 16 );
		//REQUIRE(mapToLoad->GetHeightPixels() == 16 );
		state->SetCurrentMap(mapToLoad.get(), 0);
		}
		());
	}

	Kernel::Close();
}
