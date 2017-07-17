#include "GameStateMock.h"

GameStateMock::GameStateMock(GameStateManager* gm)
	:GameState(gm){
	init=false;
	close=false;
	updateCount=0;
	drawCount=0;
}
void GameStateMock::Init(){
	init = true;
}
void GameStateMock::Close(){
	close = true;
}
void GameStateMock::HandleEvent(const Event*){
}
bool GameStateMock::Update(){
	updateCount+=1;
	return true;
}
void GameStateMock::Draw(){
	drawCount +=1;
}

EntityManager* GameStateMock::GetEntityMan(){
	return &entityMan;
}

LuaInterface* GameStateMock::GetLuaInterface(){
	return &luaInterface;
}

ComponentScriptManager* GameStateMock::GetComponentManagerScript(){
	return &comScriptMan;
}

ComponentPositionManager* GameStateMock::GetComponentManagerPosition(){
	return &comPosMan;
}

EventDispatcher* GameStateMock::GetEventDispatcher(){
	return &eventDispatcher;
}

GameStateMock::~GameStateMock(){
}
