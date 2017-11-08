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

void GS_Test::Draw() {}

// Testing
void GS_Test::ExposeTestingInterface(lua_State *state) {}

void GS_Test::AssertEqual(luabridge::LuaRef r1, luabridge::LuaRef r2) {}
void GS_Test::AssertNotEqual(luabridge::LuaRef r1, luabridge::LuaRef r2) {}
