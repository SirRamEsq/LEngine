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
	auto scriptName = "Testing/cppLuaInterfaceTest.lua";
	auto script = LScript::LoadResource(scriptName);
	auto mapDepth= 10;
	auto parent = 0;
	auto scriptType = "TEST";
	luaInterface->RunScript(eid, script.get(), mapDepth, parent, scriptName, scriptType, NULL,NULL);

	stateManager->UpdateCurrentState();

	REQUIRE(lastError == "Interface Working");

	SECTION("Ensure Input events are properly handled"){
		//script should register to listen for 'up' and 'down'
		//correct key is 'up'
		//incorrect key is 'down'
		//all else will be ignored
		auto keyPressed = "up";
		auto incorrectKey = "down";
		auto ignoredKey = "blarg"; 

		//This should not trigger any new error messages; lastError should not change by pressing a key
		auto previousErrorMessage = lastError;
		stateManager->input.SimulateKeyPress(ignoredKey);	
		REQUIRE(lastError == previousErrorMessage);

		stateManager->input.SimulateKeyPress(incorrectKey);	
		REQUIRE(lastError == "KeyPress: Incorrect Input");

		stateManager->input.SimulateKeyPress(keyPressed);	
		REQUIRE(lastError == "KeyPress: Correct Input");


		previousErrorMessage = lastError;
		stateManager->input.SimulateKeyRelease(ignoredKey);	
		REQUIRE(lastError == previousErrorMessage);

		stateManager->input.SimulateKeyRelease(incorrectKey);	
		REQUIRE(lastError == "KeyRelease: Incorrect Input");

		stateManager->input.SimulateKeyRelease(keyPressed);	
		REQUIRE(lastError == "KeyRelease: Correct Input");
	}

	SECTION("Ensure Lua events can be sent, recieved, and interpeted"){
		EID newEID = 31337;
		scriptMan->AddComponent(newEID);

		std::string eventDescription = "TEST1";

		auto scriptComponent = (ComponentScript*) scriptMan->GetComponent(eid);
		auto otherScriptComponent = (ComponentScript*) scriptMan->GetComponent(newEID);

		scriptComponent->RunFunction("Observe31337");
		REQUIRE(lastError == "Observing 31337");

		otherScriptComponent->EventLuaBroadcastEvent(eventDescription);
		REQUIRE(lastError == "EVENT: TEST1");
	}

	Kernel::Close();
}
