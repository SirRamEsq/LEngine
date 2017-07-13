#include "../catch.hpp"
#include "../../Engine/Kernel.h"
#include "../../Engine/Components/CompScript.h"
#include "../../Engine/LuaInterface.h"
#include "../../Engine/Defines.h"
#include "../mocks/GameStateMock.h"
#include "../mocks/RSC_MapMock.h"

std::string lastError = "";
EID lastErrorID = 0;
void luaErrorCallback (EID id, const std::string& error){
	lastErrorID = id;
	lastError = error;
}

TEST_CASE("Lua Interface can be instantiated", "[lua][lua_interface]"){
	Kernel::Inst();

	auto stateManager = &K_StateMan;
	auto stateAuto = make_unique<GameStateMock>(stateManager);
	auto state = stateAuto.get();
	stateManager->PushState(std::move(stateAuto));

	auto scriptMan = state->GetComponentManagerScript();
	auto posMan = state->GetComponentManagerPosition();
	auto eid = 20;

	//pos comp is prereq to having scipt comp
	posMan->AddComponent(eid);
	//script comp is prereq to luaInterface creating an entity instance
	scriptMan->AddComponent(eid);

	auto luaInterface = state->GetLuaInterface();
	luaInterface->SetErrorCallbackFunction(luaErrorCallback);
	auto scriptName = "Testing/LuaInterfaceTest.lua";
	auto script = LScript::LoadResource(scriptName);
	auto mapDepth= 10;
	auto parent = 0;
	auto scriptType = "TEST";
	luaInterface->RunScript(eid, script.get(), mapDepth, parent, scriptName, scriptType, NULL,NULL);

	stateManager->UpdateCurrentState();

	REQUIRE(lastError == "Good");
}

