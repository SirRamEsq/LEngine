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
  std::string scriptType = "Common/__BaseState.lua";
  std::vector<std::string> types;
  types.push_back(scriptType);
  MAP_DEPTH depth = 0;
  EID parent = 0;
  EID eid = EID_RESERVED_STATE_ENTITY;
  quit = false;

  if (stateScript == NULL) {
    throw LEngineException("GS_Script::Init, stateScript is NULL");
    quit = true;
    return;
  }

  auto baseScript = K_ScriptMan.GetLoadItem(scriptType, scriptType);

  comScriptMan.AddComponent(eid);
  std::vector<const RSC_Script *> scripts;
  scripts.push_back(baseScript);
  scripts.push_back(stateScript);
  luaInterface.RunScript(eid, scripts, NULL, NULL);
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
  if (mNextMap.mMap != NULL) {
    SetCurrentMap(mNextMap);
    mNextMap.Reset();
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
  return 0;
}

GS_Script *GS_Script::PushState(const std::string &scriptPath) {
  const RSC_Script *script = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
  auto newState = std::make_shared<GS_Script>(&Kernel::stateMan);

  gameStateManager->PushState(newState, script);

  // kinda sorta memory leak if all these states are kept around...
  // pushedStates.push_back(newState);

  return newState.get();
}
