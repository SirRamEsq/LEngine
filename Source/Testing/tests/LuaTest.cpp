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
	auto script = RSC_Script::LoadResource(scriptName);
	REQUIRE(script.get() != NULL);

	auto mapDepth= 10;
	auto parent = 0;
	auto scriptType = "TEST";
	luaInterface->RunScript(eid, script.get(), mapDepth, parent, scriptName, scriptType, NULL,NULL);

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
		stateManager->inputManager->SimulateKeyPress(ignoredKey);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == previousErrorMessage);

		stateManager->inputManager->SimulateKeyPress(incorrectKey);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == "KeyPress: Incorrect Input");

		stateManager->inputManager->SimulateKeyPress(keyPressed);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == "KeyPress: Correct Input");


		previousErrorMessage = lastError;
		stateManager->inputManager->SimulateKeyRelease(ignoredKey);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == previousErrorMessage);

		stateManager->inputManager->SimulateKeyRelease(incorrectKey);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == "KeyRelease: Incorrect Input");

		stateManager->inputManager->SimulateKeyRelease(keyPressed);	
		stateManager->inputManager->HandleInput();
		REQUIRE(lastError == "KeyRelease: Correct Input");
	}

	SECTION("Ensure that the LEngine module is correctly loaded"){
		auto scriptComponent = (ComponentScript*) scriptMan->GetComponent(eid);

		std::string mapDepthString = "10";
		std::string parentString = "0";
		std::stringstream eidString;
		eidString << eid;

		scriptComponent->RunFunction("PrintDepth");
		REQUIRE(lastError == mapDepthString);

		scriptComponent->RunFunction("PrintParent");
		REQUIRE(lastError == parentString);

		scriptComponent->RunFunction("PrintEID");
		REQUIRE(lastError == eidString.str());

		scriptComponent->RunFunction("PrintName");
		REQUIRE(lastError == scriptName);

		scriptComponent->RunFunction("PrintType");
		REQUIRE(lastError == scriptType);
	}

	SECTION("Ensure Lua events can be sent, recieved, and interpeted"){
		EID newEID = 31337;
		scriptMan->AddComponent(newEID);

		std::string eventDescription = "TEST1";

		auto scriptComponent = (ComponentScript*) scriptMan->GetComponent(eid);
		auto otherScriptComponent = (ComponentScript*) scriptMan->GetComponent(newEID);

		scriptComponent->RunFunction("Observe31337");
		REQUIRE(lastError == "Observing 31337");

		auto luaInterface = state->GetLuaInterface();
		luaInterface->EventLuaSendToObservers(newEID, eventDescription);
		std::stringstream ss1;
		ss1 << "EVENT: " << eventDescription;
		REQUIRE(lastError == ss1.str());
		
		//This should be the final event that scriptcomponent recieves from 31337
		std::string eventDescription2 = "Test2";
		std::stringstream ss2;
		ss2 <<  "EVENT: " << eventDescription2;
		luaInterface->EventLuaSendToObservers(newEID, eventDescription2);
		REQUIRE(lastError == ss2.str());

		//Tell newScript that the first was has been deleted, this will cause the newScript to no longer broadcast events to the first script
		Event event (eid, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "DELETED");
		state->GetEventDispatcher()->DispatchEvent(event);

		//scriptComponent should NOT recieve this event
		luaInterface->EventLuaSendToObservers(newEID, eventDescription);
		REQUIRE(lastError != ss1.str());

		//This broadcast should still reach scriptComponent though
		luaInterface->EventLuaBroadcastEvent(newEID, eventDescription);
		REQUIRE(lastError == ss1.str());

		//This direct event should also still reach scriptComponent
		luaInterface->EventLuaSendEvent(newEID, eid, eventDescription2);
		REQUIRE(lastError == ss2.str());
	}

	Kernel::Close();
}
