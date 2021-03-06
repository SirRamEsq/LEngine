#include "../../Engine/Components/CompScript.h"
#include "../../Engine/Defines.h"
#include "../../Engine/Kernel.h"
#include "../../Engine/LuaInterface.h"
#include "../GS_Test.h"

#include "../catch.hpp"
#include "../mocks/GameStateMock.h"
#include "../mocks/RSC_MapMock.h"
#include "../main.h"

#include <memory>
#include <sstream>

std::string lastError = "";
EID lastErrorID = 0;
void luaErrorCallback(EID id, const std::string &error) {
  lastErrorID = id;
  lastError = error;
}

TEST_CASE("Lua Interface can be instantiated", "[lua][lua_interface]") {
  Kernel::Inst();

  auto stateManager = &K_StateMan;
  auto stateAuto = std::make_unique<GameStateMock>(stateManager);
  auto state = stateAuto.get();
  stateManager->PushState(std::move(stateAuto));

  auto scriptMan = state->GetComponentManagerScript();
  auto posMan = state->GetComponentManagerPosition();
  auto eid = 20;

  // pos comp is prereq to having scipt comp
  posMan->AddComponent(eid);
  // script comp is prereq to luaInterface creating an entity instance
  scriptMan->AddComponent(eid);

  auto luaInterface = state->GetLuaInterface();
  luaInterface->SetErrorCallbackFunction(luaErrorCallback);
  auto scriptName = "System/Testing/cppLuaInterfaceTest.lua";
  auto script = RSC_Script::LoadResource(scriptName);
  REQUIRE(script.get() != NULL);

  auto mapDepth = 10;
  auto parent = 0;
  std::vector<const RSC_Script *> scripts;
  scripts.push_back(script.get());
  luaInterface->RunScript(eid, scripts, NULL, NULL);

  REQUIRE(lastError == "Interface Working");

  SECTION("Ensure Input events are properly handled") {
    // script should register to listen for 'up' and 'down'
    // correct key is 'up'
    // incorrect key is 'down'
    // all else will be ignored
    auto keyPressed = "up";
    auto incorrectKey = "down";
    auto ignoredKey = "blarg";

    // This should not trigger any new error messages; lastError should not
    // change by pressing a key
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

  SECTION("Ensure that the LEngine module is correctly loaded") {
    auto scriptComponent = scriptMan->GetComponent(eid);

    std::stringstream eidString;
    eidString << eid;

    std::stringstream stateEIDString;
    stateEIDString << EID_RESERVED_STATE_ENTITY;

    scriptComponent->RunFunction("PrintEID");
    REQUIRE(lastError == eidString.str());

    scriptComponent->RunFunction("PrintStateEID");
    REQUIRE(lastError == stateEIDString.str());
  }

  SECTION("Ensure Lua events can be sent, recieved, and interpeted") {
    EID newEID = 31337;
    scriptMan->AddComponent(newEID);
	scriptMan->AddNewComponents();

    std::string eventDescription = "TEST1";

    auto scriptComponent = scriptMan->GetComponent(eid);
    auto otherScriptComponent = scriptMan->GetComponent(newEID);

    scriptComponent->RunFunction("Observe31337");
    REQUIRE(lastError == "Observing 31337");

    auto luaInterface = state->GetLuaInterface();

    otherScriptComponent->BroadcastEvent(eventDescription);
    std::stringstream ss1;
    ss1 << "EVENT: " << eventDescription;
    REQUIRE(lastError == ss1.str());

    // This should be the final event that scriptcomponent recieves from 31337
    std::string eventDescription2 = "Test2";
    std::stringstream ss2;
    ss2 << "EVENT: " << eventDescription2;
    otherScriptComponent->SendEvent(eventDescription2);
    REQUIRE(lastError == ss2.str());

    // Tell newScript that the first was has been deleted, this will cause the
    // newScript to no longer broadcast events to the first script
    Event event(eid, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "DELETED");
    state->GetEventDispatcher()->DispatchEvent(event);

    // scriptComponent should NOT recieve this event
    otherScriptComponent->SendEvent(eventDescription);
    REQUIRE(lastError != ss1.str());

    // This broadcast should still reach scriptComponent though
    otherScriptComponent->BroadcastEvent(eventDescription);
    REQUIRE(lastError == ss1.str());
  }

  Kernel::Close();
}

std::unique_ptr<RSC_Script> LoadScript(const std::string &fullPath) {
  std::unique_ptr<RSC_Script> script = NULL;

  auto data = LoadGenericFile(fullPath);
  if (data.get()->GetData() == NULL) {
    return NULL;
  }
  script = std::make_unique<RSC_Script>(fullPath, data.get()->GetData(),
                                        data.get()->length);

  return script;
}

void CheckLuaAssertions(std::vector<Assertion> assertions) {
  for (auto i = assertions.begin(); i != assertions.end(); i++) {
    if (!i->mPass) {
      // Record failing assertion
      CHECK(i->mDescription == "");
    } else {
      // Record passing assertion
      CHECK(0 == 0);
    }
  }
}

TEST_CASE("Test Lua Test Harness (yes really)", "[lua]") {
  Kernel::Inst();
  std::string dir = "Resources/Scripts/System/Testing/cppTestHarnessTest.lua";

  REQUIRE(PHYSFS_getLastError() == NULL);

  InputManager inputMan;

  PHYSFS_mount(dir.c_str(), dir.c_str(), 0);

  std::vector<std::string> fileNames;
  char **rc = PHYSFS_enumerateFiles(dir.c_str());
  for (auto i = rc; *i != NULL; i++) {
    fileNames.push_back(std::string(*i));
  }
  auto script = LoadScript(dir);
  REQUIRE(script.get() != NULL);

  auto testState = std::make_shared<GS_Test>(&K_StateMan, script.get());
  K_StateMan.PushState(testState);
  Kernel::Update();

  auto assertions = testState->Test();
  int failedTests = 0;
  for (auto i = assertions.begin(); i != assertions.end(); i++) {
    if (!i->mPass) {
      failedTests++;
      CHECK(i->mDescription != "");
    }
  }

  REQUIRE(failedTests == 3);
  Kernel::Close();
}

TEST_CASE("RunLuaTests with Engine test Harness", "[lua]") {
  Kernel::Inst();

  std::string dir = LUA_TEST_DIR;
  REQUIRE(dir != "");

  REQUIRE(PHYSFS_getLastError() == NULL);
  Log testLog;
  testLog.WriteToFile("TEST_LOG_LUA");

  InputManager inputMan;

  PHYSFS_mount(dir.c_str(), dir.c_str(), 0);

  std::vector<std::string> fileNames;
  char **rc = PHYSFS_enumerateFiles(dir.c_str());
  for (auto i = rc; *i != NULL; i++) {
    fileNames.push_back(std::string(*i));
  }
  std::stringstream ss;
  ss << "Getting Files from dir '" << dir << "'";
  testLog.Write(ss.str());
  std::vector<std::unique_ptr<RSC_Script> > scripts;
  for (auto fname = fileNames.begin(); fname != fileNames.end(); fname++) {
    // If fname is a file
    testLog.Write(*fname);
    if (PHYSFS_isDirectory(fname->c_str()) == 0) {
      auto script = LoadScript(dir + "/" + *fname);
      if (script.get() != NULL) {
        testLog.Write("Pushing back script");
        scripts.push_back(std::move(script));
      } else {
        testLog.Write("SCRIPT IS NULL");
      }
    }
  }

  K_StateMan.PopState();
  Kernel::Update();
  for (auto script = scripts.begin(); script != scripts.end(); script++) {
    auto testState = std::make_shared<GS_Test>(&K_StateMan, script->get());
    K_StateMan.PushState(testState);
    Kernel::Update();

    auto assertions = testState->Test();
    CheckLuaAssertions(assertions);
    K_StateMan.PopState();
    Kernel::Update();
  }

  testLog.CloseFileHandle();
  Kernel::Close();
}
