#include "GameController.h"

GameController::GameController(SDL_GameController* pad) : mSDLpad(pad) {}

GameController::~GameController() { SDL_GameControllerClose(mSDLpad); }

void GameController::OnAxisEvent(int axisIndex, AxisCallback cb) {
  mAxisCallbacks[axisIndex] = cb;
}

void GameController::OnButtonPressEvent(int button, ButtonCallback cb) {
  mButtonPressCallbacks[button] = cb;
}

void GameController::OnButtonReleaseEvent(int button, ButtonCallback cb) {
  mButtonReleaseCallbacks[button] = cb;
}

void GameController::OnHatEvent(int hatIndex, int hatDir, ButtonCallback cb) {
  mHatCallbacks[hatIndex][hatDir] = cb;
}

void GameController::ButtonPressEvent(int button) {
  auto i = mButtonPressCallbacks.find(button);
  if (i != mButtonPressCallbacks.end()) {
    i->second();
  }
}

void GameController::ButtonReleaseEvent(int button) {
  auto i = mButtonReleaseCallbacks.find(button);
  if (i != mButtonReleaseCallbacks.end()) {
    i->second();
  }
}

void GameController::AxisEvent(int axis, int distance) {
  auto i = mAxisCallbacks.find(axis);
  if (i != mAxisCallbacks.end()) {
    i->second(distance);
  }
}

void GameController::HatEvent(int hat, int hatDir) {
  auto i = mHatCallbacks.find(hat);
  if (i != mHatCallbacks.end()) {
    auto ii = i->second.find(hatDir);
    if (ii != i->second.end()) {
      ii->second();
    }
  }
}

