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

std::string FIRST_STATE_PATH("States/startupState.lua");

GameStartState::GameStartState(GameStateManager *gsm) : GameState(gsm) {
  mCannotStart = false;
}

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
  countdown += countdownIncrement;

  auto resolution = Resolution::GetResolution();
  resolution.x = resolution.x / 3;
  resolution.y = resolution.y / 2;
  if (mCannotStart) {
    ImGui::SetNextWindowPos(resolution);
    ImGui::BeginFlags(
        "STARTUPWINDOW",
        ImGuiWindowFlags_NoScrollbar + ImGuiWindowFlags_NoTitleBar +
            ImGuiWindowFlags_AlwaysAutoResize + ImGuiWindowFlags_NoResize);
    ImGui::TextWrapper("Cannot Start");
    ImGui::TextWrapper("No script exists at path");
    std::stringstream fullPath;
    fullPath << "Data/Resources/" << FIRST_STATE_PATH;
    ImGui::TextWrapper(fullPath.str());

    ImGui::End();
  } else {
    ImGui::SetNextWindowPos(resolution);
    ImGui::BeginFlags("STARTUPWINDOW",
                      ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize);
    ImGui::TextWrapper(countdownMessage);
    ImGui::ProgressBar(countdown / countdownMax, Vec2(-1.0f, 0.0f));

    ImGui::End();
    if (countdown > countdownMax) {
      std::string scriptPath = FIRST_STATE_PATH;
      const RSC_Script *script =
          K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
      if (script != NULL) {
        gameStateManager->PushState(
            std::make_shared<GS_Script>(gameStateManager), script);
      } else {
        mCannotStart = true;
      }
    }
  }

  if (countdown < 0) {
    return false;
  }

  UpdateComponentManagers();

  return true;
}

void GameStartState::Draw() { renderMan.Render(); }
