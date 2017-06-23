#include "../catch.hpp"

#include "../../Source/StateManager.h"
#include "../../Source/Kernel.h"

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

TEST_CASE( "Can Push to, Pop from, and Empty out StateManager", "[state]" ) {
    //Pushes GameStartState
    Kernel::Inst();

    auto stateSize = K_StateMan.stackSize();
    REQUIRE(stateSize == 1);

    K_StateMan.PushState(std::move(make_unique<_test_State>(&K_StateMan)));
    stateSize = K_StateMan.stackSize();
    REQUIRE(stateSize == 2);

    K_StateMan.PopState();
    stateSize = K_StateMan.stackSize();
    REQUIRE(stateSize == 1);

    Kernel::Close();

    stateSize = K_StateMan.stackSize();
    REQUIRE(stateSize == 0);
    REQUIRE(K_StateMan.IsEmpty());
}

TEST_CASE( "State Update and Draw Called Correctly", "[state]" ) {
    Kernel::Inst();
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

    Kernel::Close();
}

TEST_CASE( "Load Map from File", "[state][LMap]"){
    //make and deploy test map
}
