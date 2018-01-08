#include "Resolution.h"

unsigned int Resolution::mResolutionX;
unsigned int Resolution::mResolutionY;
unsigned int Resolution::mVirtualResolutionX;
unsigned int Resolution::mVirtualResolutionY;

float Resolution::mVirtualAspectRatio;

Resolution::Resolution() {}

void Resolution::UpdateResolution(SDL_Window *window) {
  // int w, h;
  int display_w, display_h;
  // SDL_GetWindowSize(window, &w, &h);
  SDL_GL_GetDrawableSize(window, &display_w, &display_h);

  mResolutionX = display_w;
  mResolutionY = display_h;
}

void Resolution::SetVirtualResolution(const Vec2 &res) {
  auto newVirtualResolution = res.Round();
  mVirtualResolutionX = newVirtualResolution.x;
  mVirtualResolutionY = newVirtualResolution.y;
  mVirtualAspectRatio = mResolutionX / mResolutionY;
}

Vec2 Resolution::GetVirtualResolution() {
  return Vec2(mVirtualResolutionX, mVirtualResolutionY);
}

Vec2 Resolution::GetResolution() {
  return Vec2(mResolutionX, mResolutionY);
}
