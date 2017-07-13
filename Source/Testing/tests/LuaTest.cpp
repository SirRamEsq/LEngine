#include "../catch.hpp"
#include "../../Engine/Kernel.h"
#include "../../Engine/Components/CompScript.h"
#include "../../Engine/LuaInterface.h"
#include "../../Engine/Defines.h"
#include "../mocks/GameStateMock.h"
#include "../mocks/RSC_MapMock.h"

TEST_CASE("Lua Interface can be instantiated", "[lua][lua_interface]"){
	Kernel::Inst();

	auto stateManager = &K_StateMan;
	auto stateAuto = make_unique<GameStateMock>(stateManager);
	auto state = stateAuto.get();
	stateManager->PushState(std::move(stateAuto));

	auto luaInterface = state->GetLuaInterface();
}

