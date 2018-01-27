#ifndef L_SDL_INIT
#define L_SDL_INIT

#include "Defines.h"
#include "Errorlog.h"

#include "SDL2/SDL.h"

#include <GL/glew.h>
//#include <GL/glut.h>
#include <GL/gl.h>

#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

class SDLInit {
 public:
  static SDLInit *Inst();
  static SDLInit *pointertoself;
  static void InitSDL();
  static void CloseSDL();
  static bool InitOpenGL();

  static SDL_Window *GetWindow();

  static bool InitImgGUI(SDL_Window *window);

  static SDL_Window *mMainWindow;

 protected:
  SDLInit();

 private:
  static SDL_GLContext mMainContextGL;
};

#endif
