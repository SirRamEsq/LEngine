#ifndef LENGINE_RESOLUTION
#define LENGINE_RESOLUTION

#include "Coordinates.h"
#include "SDLInit.h"

/*
 * Class represents the screen resolution.
 * Has a pair of unsigned ints that represent what the actual
 * screen resolution is
 * and a pair of unsigned ints that represent what the desired
 * screen resolution is
 *
 * Is a singleton class, as only one screenspace will be drawn to
 */
class Resolution {
  Resolution();

 public:
  /*
   * Sets mResolutionX and mResolutionY
   * to value obtained from SDL_Window
   */
  static void UpdateResolution(SDL_Window *window);
  static Coord2df GetResolution();

  static void SetVirtualResolution(const Coord2df &res);
  static Coord2df GetVirtualResolution();

 private:
  static unsigned int mResolutionX;
  static unsigned int mResolutionY;
  static unsigned int mVirtualResolutionX;
  static unsigned int mVirtualResolutionY;

  static float mVirtualAspectRatio;
};

#endif
