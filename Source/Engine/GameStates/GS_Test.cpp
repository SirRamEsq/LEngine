#include "GS_Test.h"
#include "../Kernel.h"

GS_Test::GS_Test(GameStateManager *gsm) : GS_Script(gsm) {
  ExposeTestingInterface(luaInterface.GetState());
}

GS_Test::~GS_Test() {}

void GS_Test::Init(const RSC_Script *stateScript) {
  std::string scriptName = "STATE";
  std::string scriptType = "__BaseState";
  MAP_DEPTH depth = 0;
  EID parent = 0;
  EID eid = EID_RESERVED_STATE_ENTITY;
  quit = false;

  if (stateScript == NULL) {
    throw LEngineException("GS_Test::Init, stateScript is NULL");
    quit = true;
    return;
  }

  comScriptMan.AddComponent(eid);
  luaInterface.RunScript(eid, stateScript, depth, parent, scriptName,
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

void GS_Test::Test() {
  luabridge::LuaRef testTable = entityScript->GetScriptPointer()["TESTS"];
  ASSERT(testTable.isNil() == false);
  /*
  Table must be declared like
      table = {val1, val2, etc...}
  */
  for (int i = 0; i < testTable.length(); i++) {
    // note the i + 1 here, it's because arrays in Lua start with 1
    entityScript->RunFunction("Setup");
    luabridge::LuaRef testFunction = testTable[i + 1];
    ASSERT(testFunction.isNil() == false);
    testFunction(this);
    entityScript->RunFunction("Teardown");
  }
}

void GS_Test::Draw() {}

// Testing
void GS_Test::ExposeTestingInterface(lua_State *state) {
  using namespace luabridge;
  getGlobalNamespace(state)            // global namespace to lua
      .beginNamespace("CPP")           //'CPP' table
      .beginClass<GS_Test>("GS_Test")  // define class object
      .addFunction("Assert", &GS_Test::Assert)
      .endClass()
      .endNamespace();
}

void GS_Test::Assert(std::string message) {
  lua_Debug ar;
  auto L = luaInterface.GetState();
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  int line = ar.currentline;
	std::stringstream ss;
  ss << "Lua Testing Assertion " << std::endl
	  << "Line: " << line << std::endl
	  << "Message: '" << message << "'" << std::endl;

  std::cout << ss.str();

}
