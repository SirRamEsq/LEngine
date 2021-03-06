#include "SDLInit.h"
#include "Kernel.h"
#include "gui/imgui.h"

SDLInit *SDLInit::pointertoself = NULL;
SDLInit::SDLInit() {}
SDL_Window *SDLInit::mMainWindow = NULL;
SDL_GLContext SDLInit::mMainContextGL;

SDLInit *SDLInit::Inst() {
  pointertoself = new SDLInit();
  return pointertoself;
}

SDL_Window *SDLInit::GetWindow() { return mMainWindow; }

void SDLInit::CloseSDL() {
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();
}

bool InitImgGui(SDL_Window *window) {
  ImGuiIO &io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;  // Keyboard mapping. ImGui will use those
                                       // indices to peek into the io.KeyDown[]
                                       // array.
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDLK_a;
  io.KeyMap[ImGuiKey_C] = SDLK_c;
  io.KeyMap[ImGuiKey_V] = SDLK_v;
  io.KeyMap[ImGuiKey_X] = SDLK_x;
  io.KeyMap[ImGuiKey_Y] = SDLK_y;
  io.KeyMap[ImGuiKey_Z] = SDLK_z;

// io.RenderDrawListsFn = ImGui_ImplSdlGL3_RenderDrawLists;   // Alternatively
// you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render()
// to get the same ImDrawData pointer.
// io.SetClipboardTextFn = ImGui_ImplSdlGL3_SetClipboardText;
// io.GetClipboardTextFn = ImGui_ImplSdlGL3_GetClipboardText;
// io.ClipboardUserData = NULL;

#ifdef _WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  io.ImeWindowHandle = wmInfo.info.win.window;
#else
  (void)window;
#endif

  return true;
}

bool SDLInit::InitOpenGL() {
  glViewport(0, 0, 1024, 768);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_TEXTURE_2D);  // Enable Texture Mapping
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Bottom is the height, Top is 0
  glOrtho(0.0f, 1024, 768, 0, 0, 1);  // 2D
  // gluPerspective(45.0,(GLfloat)SCREEN_W/(GLfloat)SCREEN_H,0.1,100.0); //3D

  glClearColor(0, 0, 0, 1);

  // Initialize modelview matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  // negative values nearer to camera
  glDepthFunc(GL_GEQUAL);
  // write to depth
  glDepthMask(GL_TRUE);
  glClearDepth(0.0f);

  // Do not render any fragments with an alpha of 0.0
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.0f);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_COLOR_MATERIAL);

  SDL_GL_SetSwapInterval(1);

  if (glGetError() != GL_NO_ERROR) {
    return 0;
  }
  return 1;
}

void SDLInit::InitSDL() {
  if (SDL_Init(SDL_INIT_JOYSTICK) == -1) {
    LOG_FATAL("Didn't init SDL properly");
    return;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  mMainWindow = SDL_CreateWindow("LEngine",  // name
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 1024, 768,  // x,y,w,h
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                     SDL_WINDOW_MAXIMIZED);  // flags

  mMainContextGL = SDL_GL_CreateContext(mMainWindow);

  if (InitOpenGL() == 0) {
    LOG_FATAL("Didn't Initialize OpenGL");
    return;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    LOG_FATAL("Didn't init SDL Mixer properly");
    return;
  }
  int flags = MIX_INIT_OGG;
  int initted = Mix_Init(flags);
  if ((initted & flags) != flags) {
    std::stringstream ss;
    ss << "SDL_Mix_Init: Failed to init required ogg and mod support!"
       << "\n"
       << "SDL_Mix_Init: " << Mix_GetError();
    LOG_FATAL(ss.str());
  }

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::stringstream ss;
    ss << "Error in GLEW INIT: " << glewGetErrorString(err);
    LOG_FATAL(ss.str());
  }

  if (InitImgGui(mMainWindow) != true) {
    LOG_FATAL("Couldn't Initialize ImgGui");
  }

  SDL_GL_SetSwapInterval(1);
  //Needed?
  // SDL_EnableUNICODE(1);
}
