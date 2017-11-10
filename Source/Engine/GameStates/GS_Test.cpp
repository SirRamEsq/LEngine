#include "GS_Test.h"
#include "../Kernel.h"

Assertion::Assertion(const std::string &desc, bool p)
    : mDescription(desc), mPass(p) {}

GS_Test::GS_Test(GameStateManager *gsm, const RSC_Script *stateScript)
    : GS_Script(gsm) {
  if (stateScript == NULL) {
    throw LEngineException("GS_Test::Init, stateScript is NULL");
    quit = true;
    return;
  }

  mStateScript = stateScript;
  ExposeTestingInterface(luaInterface.GetState());
}

GS_Test::~GS_Test() {
  mCurrentTest = 0;
  mCurrentScriptName = "";
}

void GS_Test::Init(const RSC_Script *ignore) {
  std::string scriptName = "STATE";
  std::string scriptType = "__BaseState";
  MAP_DEPTH depth = 0;
  EID parent = 0;
  EID eid = EID_RESERVED_STATE_ENTITY;
  quit = false;

  comScriptMan.AddComponent(eid);
  luaInterface.RunScript(eid, mStateScript, depth, parent, scriptName,
                         scriptType, NULL, NULL);
  entityScript = comScriptMan.GetComponent(eid);
}

void GS_Test::HandleEvent(const Event *event) {
  if (event->message == Event::MSG::KEYDOWN) {
    std::string inputKey = event->description;

    if (inputKey == "use") {
      quit = true;
    }
  }

  else if (event->message == Event::MSG::ENTITY_DELETED) {
    if (event->sender == EID_RESERVED_STATE_ENTITY) {
      entityScript = NULL;
      quit = true;
    }
  }
}

void GS_Test::KernelUpdate() { Kernel::Update(); }

/*
GS_TEST will simply use the inhereted function
bool GS_Test::Update() {
  if (nextMap != NULL) {
    auto tempMap = nextMap;
    auto tempEntrance = nextMapEntrance;
    SetCurrentMap(tempMap, tempEntrance);
    nextMap = NULL;
    nextMapEntrance = 0;
  }
  UpdateComponentManagers();
  return !quit;
}
*/

std::vector<Assertion> GS_Test::Test() {
  mCurrentTestAssertions.clear();
  luabridge::LuaRef testTable = entityScript->GetScriptPointer()["TESTS"];
  ASSERT(testTable.isNil() == false);
  mCurrentScriptName = entityScript->scriptName;
  auto setupFunction = entityScript->GetFunction("Setup");
  auto teardownFunction = entityScript->GetFunction("Teardown");
  /*
  Table must be declared like
      table = {val1, val2, etc...}
  */
  for (int i = 0; i < testTable.length(); i++) {
    // note the i + 1 here, it's because arrays in Lua start with 1
    mCurrentTest = i;
    luabridge::LuaRef testFunction = testTable[i + 1];
    ASSERT(testFunction.isNil() == false);

    setupFunction(this);
    testFunction(this);
    teardownFunction(this);
  }
  return mCurrentTestAssertions;
}

void GS_Test::Draw() {}

// Testing
void GS_Test::ExposeTestingInterface(lua_State *state) {
  using namespace luabridge;
  getGlobalNamespace(state)            // global namespace to lua
      .beginNamespace("CPP")           //'CPP' table
      .beginClass<GS_Test>("GS_Test")  // define class object
      .addFunction("Error", &GS_Test::Error)
      .addFunction("Update", &GS_Test::KernelUpdate)
      .addFunction("REQUIRE_EQUAL", &GS_Test::REQUIRE_EQUAL)
      .addFunction("REQUIRE_NOT_EQUAL", &GS_Test::REQUIRE_NOT_EQUAL)
      .endClass()
      .endNamespace();
}

bool GS_Test::REQUIRE_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2) {
  if (r1 == r2) {
    Pass("r1 == r2");
    return true;
  }

  Error("r1 != r2");
  return false;
}

bool GS_Test::REQUIRE_NOT_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2) {
  if (r1 == r2) {
    Error("r1 == r2");
    return false;
  }
  Pass("r1 != r2");
  return true;
}

std::string GS_Test::GenerateAssertionString(const std::string &message) {
  lua_Debug ar;
  auto L = luaInterface.GetState();
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  int line = ar.currentline;
  std::stringstream ss;
  ss << "Lua Testing Assertion " << std::endl
     << "ScriptName: " << mCurrentScriptName << std::endl
     << "TestFunction#: " << mCurrentTest << std::endl
     << "Line: " << line << std::endl
     << "Message: '" << message << "'";

  return ss.str();
}

void GS_Test::Error(const std::string &message) {
  auto debugInfo = GenerateAssertionString(message);
  mCurrentTestAssertions.push_back(Assertion(debugInfo, false));
}

void GS_Test::Pass(const std::string &message) {
  auto debugInfo = GenerateAssertionString(message);
  mCurrentTestAssertions.push_back(Assertion(debugInfo, true));
}
