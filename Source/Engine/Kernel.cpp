#include "Kernel.h"
#include "Resolution.h"
#include "gui/imgui_LEngine.h"

Log *Kernel::log = &Log::staticLog;

SDLInit *Kernel::SDLMan;
InputManager Kernel::inputMan;
CommandLineArgs Kernel::commandLine;
AudioSubsystem Kernel::audioSubsystem;
InputManager Kernel::inputManager;
GameStateManager_Impl Kernel::stateMan(&Kernel::inputManager);

int Kernel::gameLoops;
unsigned int Kernel::nextGameTick;
int Kernel::returnValue;
bool Kernel::debugMode;
bool Kernel::debugNextFrame;
bool Kernel::debugPause;
std::vector<bool> Kernel::debugLogFlags;

GenericContainer<RSC_Sprite> Kernel::rscSpriteMan;
GenericContainer<RSC_Texture> Kernel::rscTexMan;
GenericContainer<RSC_Sound> Kernel::rscSoundMan;
GenericContainer<RSC_Music> Kernel::rscMusicMan;
GenericContainer<RSC_Script> Kernel::rscScriptMan;
GenericContainer<RSC_Map> Kernel::rscMapMan;
GenericContainer<RSC_GLShader> Kernel::rscShaderMan;
GenericContainer<RSC_GLProgram> Kernel::rscShaderProgramMan;
GenericContainer<RSC_Font> Kernel::rscFontMan;

ImGuiState Kernel::guiState;

Kernel::Kernel() {}
Kernel::~Kernel() {}

void ImGuiState::Reset() {
  time = 0.0f;
  Kernel::ImGuiInvalidateFontTexture();
  fontTexture = 0;

  shaderHandle.reset(NULL);
  vertHandle.reset(NULL);
  fragHandle.reset(NULL);

  glDeleteBuffers(1, &vboHandle);
  glDeleteBuffers(1, &elementsHandle);

  glDeleteVertexArrays(1, &vaoHandle);

  attribLocationUV = 0;
  attribLocationProjMtx = 0;
  attribLocationPosition = 0;
  attribLocationTex = 0;
  attribLocationColor = 0;

  vboHandle = 0;
  vaoHandle = 0;
  elementsHandle = 0;
}

void Kernel::Close() {
  log->Write("Closing");
  stateMan.Close();
  SDLMan->CloseSDL();

  rscTexMan.Clear();
  rscSpriteMan.Clear();
  rscMusicMan.Clear();
  rscSoundMan.Clear();
  rscScriptMan.Clear();
  rscMapMan.Clear();
  rscShaderMan.Clear();
  rscShaderProgramMan.Clear();
  rscFontMan.Clear();

  guiState.Reset();

  ImGui::Shutdown();
}

void Kernel::Inst() {
  int argc = 0;
  char *argv[0];
  Kernel::Inst(argc, argv);
}
void Kernel::Inst(int argc, char *argv[]) {
  PHYSFS_init(NULL);
  std::string searchPath = "Data/";
  PHYSFS_addToSearchPath(searchPath.c_str(), 0);
  PHYSFS_setWriteDir("Data/");

#ifndef DEBUG_MODE
  log->WriteToFile("Log");
  debugMode = false;
#else
  log->WriteToFile("Log_DEBUG");
  debugMode = true;
#endif
  log->Write("Starting up...");

  const char *physfsError = PHYSFS_getLastError();
  if (physfsError != NULL) {
    std::stringstream ss;
    ss << "Physfs Error in Kernel Inst; Error: " << physfsError;
    log->Write(ss.str());
  }

  // commandLine.ParseArgs(argc, argv);

  if (debugMode) {
    for (auto i = Log::SEVERITY_STR.begin(); i != Log::SEVERITY_STR.end();
         i++) {
      debugLogFlags.push_back(true);
    }
    auto *fp = static_cast<bool (*)(const Log::Entry &, int, int)>(
        [](const Log::Entry &entry, int flags, int count) {
          // Cut down on rendering, only display the most recent 200 messages
          if (count > 200) {
            return false;
          }
          if ((flags & entry.severity) == 0) {
            return false;
          }

          return true;
        });
    log->SetEntryFilter(fp);
  }
  debugPause = false;
  SDLMan = SDLInit::Inst();
  SDLMan->InitSDL();
  /// \TODO remove this, have InitSDL intialize everything
  SDLMan->InitOpenGL();

  Resolution::UpdateResolution(SDLMan->mMainWindow);
  Resolution::SetVirtualResolution(Coord2df(480, 320));

  rscTexMan.SetLoadFunction(&RSC_Texture::LoadResource);
  rscSpriteMan.SetLoadFunction(&RSC_Sprite::LoadResource);
  rscMusicMan.SetLoadFunction(&RSC_Music::LoadResource);
  rscSoundMan.SetLoadFunction(&RSC_Sound::LoadResource);
  rscScriptMan.SetLoadFunction(&RSC_Script::LoadResource);
  rscMapMan.SetLoadFunction(&RSC_MapImpl::LoadResource);
  rscFontMan.SetLoadFunction(&RSC_Font::LoadResource);

  rscTexMan.SetLog(log);
  rscSpriteMan.SetLog(log);
  rscMusicMan.SetLog(log);
  rscSoundMan.SetLog(log);
  rscScriptMan.SetLog(log);
  rscMapMan.SetLog(log);
  rscFontMan.SetLog(log);

  gameLoops = 0;
  nextGameTick = SDL_GetTicks() - 1;

  Kernel::stateMan.PushState(
      std::make_unique<GameStartState>(&Kernel::stateMan));
  Kernel::stateMan.PushNextState();
}

bool Kernel::IsInDebugMode() { return debugMode; }

void Kernel::DEBUG_DebugWindowBegin() {
  ImGui::Begin("DEBUG");
  debugNextFrame = false;
  if (ImGui::Button("Pause")) {
    debugPause = not debugPause;
  }
  ImGui::SameLine();
  if (ImGui::Button("NextFrame")) {
    debugNextFrame = true;
  }
}

void Kernel::DEBUG_DebugWindowEnd() { ImGui::End(); }

void Kernel::DEBUG_DisplayLog() {
  if (ImGui::CollapsingHeader("Log")) {
    int newFlags = 0;
    int index = 0;
    for (auto i = Log::SEVERITY_STR.begin(); i != Log::SEVERITY_STR.end();
         i++) {
      bool pressed = debugLogFlags[index];
      ImGui::Checkbox(i->second.c_str(), &pressed);
      debugLogFlags[index] = pressed;
      if (pressed) {
        newFlags += i->first;
      }
      index++;
    }

    log->SetEntryFilterFlags(newFlags);
    auto entries = log->GetEntries();

    for (auto i = entries.begin(); i != entries.end(); i++) {
      ImGui::TextWrapped((*i)->ToString().c_str());
    }
  }
}

bool Kernel::Update() {
  inputManager.HandleInput();

  Resolution::UpdateResolution(SDLMan->mMainWindow);
  ImGuiNewFrame(SDLMan->mMainWindow);

  if (debugMode) {
    DEBUG_DebugWindowBegin();
    DEBUG_DisplayLog();
    DEBUG_DebugWindowEnd();
  }

  nextGameTick = SDL_GetTicks() + SKIP_TICKS;
  if ((not debugPause) or (debugNextFrame)) {
    returnValue = stateMan.Update();
  }

  // Audio subsystem can be put on a different thread
  // although with sdlMixer, it already is on a different thread
  audioSubsystem.ProcessEvents();

  gameLoops++;

  // If we're behind, skip drawing
  // Don't skip if the max amount of frame skip has been passed
  if ((SDL_GetTicks() < nextGameTick) or (gameLoops > MAX_FRAMESKIP)) {
    // game render
    stateMan.Draw();
    glFinish();
  }
  SDL_GL_SwapWindow(SDLMan->GetWindow());

  return returnValue;
}

bool Kernel::Run() {
  gameLoops = 0;

  // loop seems to be locked to 60fps no matter what?
  while (SDL_GetTicks() > nextGameTick) {
    if (Update() != true) {
      return false;
    }
  }
  return true;
}

void Kernel::ImGuiCreateDeviceObjects() {
  // Backup GL state
  GLint last_texture, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  if (guiState.shaderHandle.get() == NULL) {
    const GLchar *vertex_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar *fragment_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    guiState.vertHandle =
        std::make_unique<RSC_GLShader>(vertex_shader, SHADER_VERTEX);
    guiState.fragHandle =
        std::make_unique<RSC_GLShader>(fragment_shader, SHADER_FRAGMENT);
    guiState.shaderHandle = std::make_unique<RSC_GLProgram>();

    if (guiState.vertHandle->IsUsable() == false) {
      log->Write("Couldn't load ImGui Vertex Shader");
      throw LEngineException("Imgui No Vertex Shader");
    }
    if (guiState.fragHandle->IsUsable() == false) {
      log->Write("Couldn't load ImGui Fragment Shader");
      throw LEngineException("Imgui No Fragment Shader");
    }

    guiState.shaderHandle->AddShader(guiState.vertHandle.get());
    guiState.shaderHandle->AddShader(guiState.fragHandle.get());
    guiState.shaderHandle->LinkProgram();
    guiState.shaderHandle->Bind();

    guiState.attribLocationTex =
        glGetUniformLocation(guiState.shaderHandle->GetHandle(), "Texture");
    guiState.attribLocationProjMtx =
        glGetUniformLocation(guiState.shaderHandle->GetHandle(), "ProjMtx");
    guiState.attribLocationPosition =
        glGetAttribLocation(guiState.shaderHandle->GetHandle(), "Position");
    guiState.attribLocationUV =
        glGetAttribLocation(guiState.shaderHandle->GetHandle(), "UV");
    guiState.attribLocationColor =
        glGetAttribLocation(guiState.shaderHandle->GetHandle(), "Color");

    glGenBuffers(1, &guiState.vboHandle);
    glGenBuffers(1, &guiState.elementsHandle);

    glGenVertexArrays(1, &guiState.vaoHandle);
    glBindVertexArray(guiState.vaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, guiState.vboHandle);
    glEnableVertexAttribArray(guiState.attribLocationPosition);
    glEnableVertexAttribArray(guiState.attribLocationUV);
    glEnableVertexAttribArray(guiState.attribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(guiState.attribLocationPosition, 2, GL_FLOAT,
                          GL_FALSE, sizeof(ImDrawVert),
                          (GLvoid *)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(guiState.attribLocationUV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid *)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(guiState.attribLocationColor, 4, GL_UNSIGNED_BYTE,
                          GL_TRUE, sizeof(ImDrawVert),
                          (GLvoid *)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF
  }

  if (guiState.fontTexture == 0) {
    ImGuiCreateFontsTexture();
  }

  // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindVertexArray(last_vertex_array);
}

void Kernel::ImGuiCreateFontsTexture() {
  // Build texture atlas
  ImGuiIO &io = ImGui::GetIO();
  unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(
      &pixels, &width, &height);  // Load as RGBA 32-bits for OpenGL3 demo
                                  // because it is more likely to be compatible
                                  // with user's existing shader.

  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &guiState.fontTexture);
  glBindTexture(GL_TEXTURE_2D, guiState.fontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t)guiState.fontTexture;

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);
}

void Kernel::ImGuiNewFrame(SDL_Window *window) {
  if (!guiState.fontTexture) {
    ImGuiCreateDeviceObjects();
  }

  ImGuiIO &io = ImGui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  SDL_GetWindowSize(window, &w, &h);
  SDL_GL_GetDrawableSize(window, &display_w, &display_h);
  io.DisplaySize = ImVec2((float)w, (float)h);
  io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                      h > 0 ? ((float)display_h / h) : 0);
  guiState.projectionMatrix =
      Matrix4::OrthoGraphicProjectionMatrix(Coord2df(w, h));

  // Setup time step
  Uint32 time = SDL_GetTicks();
  double current_time = time / 1000.0;
  io.DeltaTime = guiState.time > 0.0 ? (float)(current_time - guiState.time)
                                     : (float)(1.0f / 60.0f);
  guiState.time = current_time;

  // Setup inputs
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
    io.MousePos = ImVec2(inputManager.GetMousePosition());
  } else {
    io.MousePos = ImVec2(-1, -1);
  }

  io.MouseDown[0] = inputManager.GetMouseButtonLeft();
  io.MouseDown[1] = inputManager.GetMouseButtonRight();
  io.MouseDown[2] = inputManager.GetMouseButtonMiddle();
  io.MouseWheel = inputManager.GetMouseWheel();

  // Hide OS mouse cursor if ImGui is drawing it
  SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

  // Start the frame
  ImGui::NewFrame();
}

void Kernel::ImGuiInvalidateFontTexture() {
  // Delete Font texture and it will be reloaded next frame
  if (Kernel::guiState.fontTexture != 0) {
    glDeleteTextures(1, &Kernel::guiState.fontTexture);
    Kernel::guiState.fontTexture = 0;
  }
}
