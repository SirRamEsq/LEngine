#include "GS_Script.h"

GS_Script::GS_Script(GameStateManager* gsm)
	:GameState(gsm){

}
//Initialize states with an optional script? RSC_Script
void GS_Script::Init(const RSC_Script* stateScript){
	std::string scriptName = "STATE";
	std::string scriptType = "gui";
	MAP_DEPTH depth = 0;
	EID parent = 0;
	EID eid = EID_RESERVED_STATE_ENTITY;

	if(stateScript==NULL){
		throw LEngineException("GS_Script::Init, stateScript is NULL");
	}

	//comPosMan.AddComponent(eid);
	comScriptMan.AddComponent(eid);

	luaInterface.RunScript(eid, stateScript, depth, parent, scriptName, scriptType, NULL, NULL);

	quit = false;
}

void GS_Script::Close(){

}

void GS_Script::HandleEvent(const Event* event){
    if(event->message == Event::MSG::KEYDOWN){
		std::string inputKey = event->description;

        if     ( inputKey == "use")    {quit=true;}
    }

	else if(event->message == Event::MSG::ENTITY_DELETED){
			quit = true;
	}
}

bool GS_Script::Update(){
	if(nextMap != NULL){
		SetCurrentMap(nextMap, nextMapEntrance);
		nextMap = NULL;
		nextMapEntrance = NULL;
	}
	UpdateComponentManagers();
	return !quit;
}

void GS_Script::Draw(){
	renderMan.Render();
}
