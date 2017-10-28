#include "GS_Script.h"
#include "../Kernel.h"

GS_Script::GS_Script(GameStateManager *gsm) : GameState(gsm) {
  isLuaState = true;
}

GS_Script::~GS_Script() {
  entityMan.ClearAllEntities();
  entityMan.ClearAllReservedEntities();
}

void GS_Script::Init(const RSC_Script *stateScript) {
  std::string scriptName = "STATE";
  std::string scriptType = "__BaseState";
  MAP_DEPTH depth = 0;
  EID parent = 0;
  EID eid = EID_RESERVED_STATE_ENTITY;
  quit = false;

  if (stateScript == NULL) {
    throw LEngineException("GS_Script::Init, stateScript is NULL");
    quit = true;
    return;
  }

  comScriptMan.AddComponent(eid);
  luaInterface.RunScript(eid, stateScript, depth, parent, scriptName,
                         scriptType, NULL, NULL);
  entityScript = comScriptMan.GetComponent(eid);
}

void GS_Script::Close() {
  if (entityScript != NULL) {
    entityScript->RunFunction("Close");
  }
}

void GS_Script::Resume() {
  if (entityScript != NULL) {
    entityScript->RunFunction("Resume");
  }
}

void GS_Script::HandleEvent(const Event *event) {
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

bool GS_Script::Update() {
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

void GS_Script::Draw() { renderMan.Render(); }

////////////////////
// SCRIPT INTERFACE//
////////////////////
EID GS_Script::GetStateEID() const {
  if (entityScript != NULL) {
    entityScript->GetEID();
  }
}

GS_Script *GS_Script::PushState(const std::string &scriptPath) {
  const RSC_Script *script = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
  auto newState = std::make_shared<GS_Script>(&Kernel::stateMan);

  gameStateManager->PushState(newState, script);

  pushedStates.push_back(newState);

  return newState.get();
}
