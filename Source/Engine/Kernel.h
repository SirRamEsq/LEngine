#ifndef L_KERNEL
#define L_KERNEL

#include "BaseComponent.h"
#include "Errorlog.h"
#include "GenericContainer.h"
#include "IniHandler.h"
#include "Input.h"
#include "RenderManager.h"
#include "SDLInit.h"
#include "StateManager.h"

#include "GameStates/GameStart.h"

#include "Resources/RSC_Font.h"
#include "Resources/RSC_GLShader.h"
#include "Resources/RSC_Heightmap.h"
#include "Resources/RSC_Map.h"
#include "Resources/RSC_Script.h"
#include "Resources/RSC_Sound.h"
#include "Resources/RSC_Sprite.h"
#include "Resources/RSC_Texture.h"
#include "Resources/RSC_Prefab.h"
#include "Resources/ResourceLoading.h"

#include "AudioSubsystem.h"
#include "CommandLineArgs.h"

#include <SOIL/SOIL.h>

#include <vector>

struct ImGuiState {
  void Reset();

  double time = 0.0f;
  GLuint fontTexture = 0;

  std::unique_ptr<RSC_GLProgram> shaderHandle;
  std::unique_ptr<RSC_GLShader> vertHandle;
  std::unique_ptr<RSC_GLShader> fragHandle;

  int attribLocationTex = 0;
  int attribLocationProjMtx = 0;
  int attribLocationPosition = 0;
  int attribLocationUV = 0;
  int attribLocationColor = 0;

  unsigned int vboHandle = 0;
  unsigned int vaoHandle = 0;
  unsigned int elementsHandle = 0;

  Matrix4 projectionMatrix;
};

class Kernel {
 public:
  static void Inst(int argc, char *argv[]);
  static void Inst();
  static bool Run();
  static bool Update();
  static void Close();

  static AudioSubsystem audioSubsystem;
  static GameStateManager_Impl stateMan;
  static InputManager inputMan;
  static CommandLineArgs commandLine;

  static GenericContainer<RSC_Sprite> rscSpriteMan;
  static GenericContainer<RSC_Sound> rscSoundMan;
  static GenericContainer<RSC_Music> rscMusicMan;
  static GenericContainer<RSC_Script> rscScriptMan;
  static GenericContainer<RSC_Texture> rscTexMan;
  static GenericContainer<RSC_Prefab> rscPrefabMan;
  static GenericContainer<RSC_Map> rscMapMan;
  static GenericContainer<RSC_Font> rscFontMan;

  // RSC_GLPrograms don't own shaders, but obviously make extensive use of them
  // be careful not to delete shaders in use by programs
  static GenericContainer<RSC_GLShader> rscShaderMan;
  static GenericContainer<RSC_GLProgram> rscShaderProgramMan;

  static InputManager inputManager;
  static Log *log;

  static ImGuiState guiState;

  static bool IsInDebugMode();

  static void ImGuiCreateFontsTexture();
  static void ImGuiInvalidateFontTexture();

 private:
  static bool debugMode;
  static bool debugPause;
  static bool debugNextFrame;

  Kernel();
  ~Kernel();

  /// Is called at the start of every new frame for the sake of imgGui
  static void ImGuiNewFrame(SDL_Window *window);
  /// Is called once per run to initialize imgui resources
  static void ImGuiCreateDeviceObjects();

  static int gameLoops;
  static unsigned int nextGameTick;

  static int returnValue;
  static SDLInit *SDLMan;

  static void DEBUG_DebugWindowBegin();
  static void DEBUG_DisplayLog();
  static void DEBUG_DebugWindowEnd();

  static std::vector<bool> debugLogFlags;
};

// for more laconic access
#define K_StateMan Kernel::stateMan
#define K_CommandLine Kernel::commandLine
#define K_AudioSub Kernel::audioSubsystem

#define K_TextureMan Kernel::rscTexMan
#define K_SpriteMan Kernel::rscSpriteMan
#define K_SoundMan Kernel::rscSoundMan
#define K_MusicMan Kernel::rscMusicMan
#define K_ScriptMan Kernel::rscScriptMan
#define K_PrefabMan Kernel::rscPrefabMan
#define K_MapMan Kernel::rscMapMan
#define K_FontMan Kernel::rscFontMan
#define K_ShaderMan Kernel::rscShaderMan
#define K_ShaderProgramMan Kernel::rscShaderProgramMan

#endif
