#include "GamePause.h"
#include "../Kernel.h"
RenderText *tt = NULL;
RenderText *tt1 = NULL;
RenderText *tt2 = NULL;
RenderText *tt3 = NULL;
RenderText *tt4 = NULL;
int ttid = 0;
int ttid1 = 0;
int ttid2 = 0;
int ttid3 = 0;
int ttid4 = 0;

GamePauseState::GamePauseState(GameStateManager *gsm) : GameState(gsm) {}

GamePauseState::~GamePauseState() {
  ttid = 0;
  delete tt;
  tt = NULL;
  ttid1 = 0;
  delete tt1;
  tt1 = NULL;
  ttid2 = 0;
  delete tt2;
  tt2 = NULL;
  ttid3 = 0;
  delete tt3;
  tt3 = NULL;
  ttid4 = 0;
  delete tt4;
  tt4 = NULL;
}

void GamePauseState::Init(const RSC_Script *stateScript) {
  exit = false;

  tt = new RenderText(&renderMan, 100, 100, "| Tha Game is Paused, Dawg |",
                      false);
  tt->SetColorI(255, 250, 250);

  tt1 = new RenderText(&renderMan, 99, 100, "| Tha Game is Paused, Dawg |",
                       false);
  tt1->SetColorI(0, 0, 0);

  tt2 = new RenderText(&renderMan, 101, 100, "| Tha Game is Paused, Dawg |",
                       false);
  tt2->SetColorI(0, 0, 0);

  tt3 = new RenderText(&renderMan, 100, 99, "| Tha Game is Paused, Dawg |",
                       false);
  tt3->SetColorI(0, 0, 0);

  tt4 = new RenderText(&renderMan, 100, 101, "| Tha Game is Paused, Dawg |",
                       false);
  tt4->SetColorI(0, 0, 0);

  tt1->SetDepth(-1000);
  tt2->SetDepth(-1000);
  tt3->SetDepth(-1000);
  tt4->SetDepth(-1000);
  tt->SetDepth(-1001);
}

void GamePauseState::Close() {}

void GamePauseState::Resume() {}

void GamePauseState::HandleEvent(const Event *event) {
  if (event->message == Event::MSG::KEYDOWN) {
    std::string inputKey = event->description;
    if (inputKey == "pause") {
      exit = 1;
    }
  }
}

bool GamePauseState::Update() {
  if (exit) {
    return false;
  }

  return true;
}

void GamePauseState::Draw() {
  // DrawPreviousState();
  renderMan.Render();
}
