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

void Resolution::SetVirtualResolution(const Coord2df &res) {
  auto newVirtualResolution = res.Round();
  mVirtualResolutionX = newVirtualResolution.x;
  mVirtualResolutionY = newVirtualResolution.y;
  mVirtualAspectRatio = mResolutionX / mResolutionY;
}

Coord2df Resolution::GetVirtualResolution() {
  return Coord2df(mVirtualResolutionX, mVirtualResolutionY);
}

Coord2df Resolution::GetResolution() {
  return Coord2df(mResolutionX, mResolutionY);
}
