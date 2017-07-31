#include "GameStart.h"
#include "../Kernel.h"
#include "../RenderManager.h"


GameStartState::GameStartState(GameStateManager* gsm)
	: GameState(gsm){

 }

void GameStartState::Init(){
	std::string scriptPath = "GUI/mainMenu.lua";
	std::string scriptType = "gui";
	MAP_DEPTH depth = 0;
	EID parent = 0;
	EID eid = entityMan.NewEntity();
	const RSC_Script* guiScript = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);

	if(guiScript==NULL){
		throw LEngineException("Couldn't load GameStartState gui script");
	}

	comPosMan.AddComponent(eid);
	comScriptMan.AddComponent(eid);

	luaInterface.RunScript(eid, guiScript, depth, parent, scriptPath, scriptType, NULL, NULL);
}

void GameStartState::Close(){
}

int countdown = 1;

void GameStartState::HandleEvent(const Event* event){
    if(event->message == Event::MSG::KEYDOWN){
		std::string inputKey = event->description;

        if     ( inputKey == "use")    {countdown=-1;}
//        else if( inputKey == "pause")  {gameStateManager->PushState(make_unique<GamePauseState> (gameStateManager));}
    }
}

bool GameStartState::Update(){
    if(countdown < 0){return false;}

    UpdateComponentManagers();

    return true;
}

void GameStartState::Draw(){
    renderMan.Render();
}
