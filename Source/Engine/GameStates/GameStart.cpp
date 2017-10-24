#include "GameStart.h"
#include "GS_Script.h"

#include "../Kernel.h"
#include "../RenderManager.h"
#include "../Resolution.h"
#include "../gui/imgui_LEngine.h"

int countdown = 0;
float countdownMax = 100;
float countdownIncrement = 2.5;
std::string countdownMessage = "~----- Starting Up! -----~";

GameStartState::GameStartState(GameStateManager *gsm) : GameState(gsm) {}

GameStartState::~GameStartState() {}

void GameStartState::Init(const RSC_Script *stateScript) {}

void GameStartState::Close() {}

void GameStartState::Resume() {
  countdownIncrement *= -1;
  countdownMessage = "~----- Shutting Down! -----~";
}

void GameStartState::HandleEvent(const Event *event) {
  if (event->message == Event::MSG::KEYDOWN) {
    std::string inputKey = event->description;

    if (inputKey == "use") {
      countdown = -100;
    }
  }
}

bool GameStartState::Update() {
  auto resolution = Resolution::GetResolution();
  resolution.x -= 100;
  resolution.y -= 50;
  ImGui::SetNextWindowPos(resolution);
  ImGui::BeginFlags("STARTUPWINDOW",
                    ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize);
  ImGui::TextWrapper(countdownMessage);
  ImGui::ProgressBar(countdown / countdownMax, Coord2df(-1.0f, 0.0f));

  ImGui::End();
  if (countdown < 0) {
    return false;
  }
  countdown += countdownIncrement;
  if (countdown > countdownMax) {
    std::string scriptPath = "States/startupState.lua";
    const RSC_Script *script = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
    gameStateManager->PushState(std::make_shared<GS_Script>(gameStateManager),
                                script);
  }

  UpdateComponentManagers();

  return true;
}

void GameStartState::Draw() { renderMan.Render(); }
