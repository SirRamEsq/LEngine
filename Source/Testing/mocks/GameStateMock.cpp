#include "GameStateMock.h"

GameStateMock::GameStateMock(GameStateManager *gm) : GameState(gm) {
  init = false;
  resume = false;
  close = false;
  updateCount = 0;
  drawCount = 0;
  mNextMap.Reset();
}
void GameStateMock::Init(const RSC_Script *stateScript) { init = true; }
void GameStateMock::Close() { close = true; }
void GameStateMock::Resume() { resume = true; }
void GameStateMock::HandleEvent(const Event *) {}
bool GameStateMock::Update() {
  if (mNextMap.mMap != NULL) {
    SetCurrentMap(mNextMap);
    mNextMap.Reset();
  }
  updateCount += 1;
  return true;
}
void GameStateMock::Draw() { drawCount += 1; }

EntityManager *GameStateMock::GetEntityMan() { return &entityMan; }

LuaInterface *GameStateMock::GetLuaInterface() { return &luaInterface; }

ComponentScriptManager *GameStateMock::GetComponentManagerScript() {
  return &comScriptMan;
}

ComponentPositionManager *GameStateMock::GetComponentManagerPosition() {
  return &comPosMan;
}

EventDispatcher *GameStateMock::GetEventDispatcher() {
  return &eventDispatcher;
}

GameStateMock::~GameStateMock() {}
