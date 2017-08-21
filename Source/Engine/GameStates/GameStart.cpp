#include "GameStart.h"
#include "GS_Script.h"

#include "../Kernel.h"
#include "../RenderManager.h"
#include "../gui/imgui_LEngine.h"


int countdown = 0;
int countdownMax = 100;
int countdownIncrement = 1;
std::string countdownMessage = "~----- Starting Up! -----~";

GameStartState::GameStartState(GameStateManager* gsm)
	: GameState(gsm){

 }

GameStartState::~GameStartState(){

}

void GameStartState::Init(const RSC_Script* stateScript){

}

void GameStartState::Close(){

}

void GameStartState::Resume(){
	countdownIncrement *= -1;
	countdownMessage = "~----- Shutting Down! -----~";
}


void GameStartState::HandleEvent(const Event* event){
    if(event->message == Event::MSG::KEYDOWN){
		std::string inputKey = event->description;

        if     ( inputKey == "use")    {countdown=-100;}
//        else if( inputKey == "pause")  {gameStateManager->PushState(make_unique<GamePauseState> (gameStateManager));}
    }
}

bool GameStartState::Update(){
	ImGui::SetNextWindowPos(Coord2df((SCREEN_W/2)-100, (SCREEN_H/2)-50));
	ImGui::BeginFlags("STARTUPWINDOW", ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize);
	ImGui::TextWrapper(countdownMessage);
	ImGui::ProgressBar(float(countdown) / float(countdownMax), Coord2df(-1.0f, 0.0f));

	ImGui::End();
    if(countdown < 0){return false;}
	countdown += countdownIncrement;
	if(countdown > countdownMax){
		std::string scriptPath = "States/startupState.lua";
		const RSC_Script* script = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
		gameStateManager->PushState(make_unique<GS_Script>(gameStateManager), script);
	}

    UpdateComponentManagers();

    return true;
}

void GameStartState::Draw(){
    renderMan.Render();
}
