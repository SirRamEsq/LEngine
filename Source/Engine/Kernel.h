#ifndef L_KERNEL
#define L_KERNEL

#include "Errorlog.h"
#include "BaseComponent.h"
#include "GenericContainer.h"
#include "StateManager.h"
#include "SDLInit.h"
#include "IniHandler.h"
#include "Input.h"
#include "RenderManager.h"

#include "GameStates/GameStart.h"

#include "Resources/ResourceLoading.h"
#include "Resources/RSC_Texture.h"
#include "Resources/RSC_Sprite.h"
#include "Resources/RSC_Sound.h"
#include "Resources/RSC_Script.h"
#include "Resources/RSC_Heightmap.h"
#include "Resources/RSC_Background.h"
#include "Resources/RSC_Map.h"
#include "Resources/RSC_GLShader.h"
#include "Resources/RSC_Font.h"

#include "AudioSubsystem.h"
#include "CommandLineArgs.h"

#include <SOIL/SOIL.h>

#include <vector>

struct ImGuiState{
	bool 	mousePressed[3] = { false, false, false };
	float 	mouseWheel = 0.0f;

	double 	time = 0.0f;
	GLuint 	fontTexture = 0;

	std::unique_ptr<RSC_GLProgram> 	shaderHandle;
	std::unique_ptr<RSC_GLShader> 	vertHandle;
	std::unique_ptr<RSC_GLShader> 	fragHandle;

	int 	attribLocationTex = 0;
	int 	attribLocationProjMtx = 0;
	int 	attribLocationPosition = 0;
	int 	attribLocationUV = 0;
	int 	attribLocationColor = 0;

	unsigned int 	vboHandle = 0;
	unsigned int	vaoHandle = 0;
	unsigned int	elementsHandle = 0;

	Matrix4 projectionMatrix;
};

class Kernel{
	public:
		static void Inst(int argc, char *argv[]);
		static void Inst();
		static bool Run();
		static bool Update();
		static void Close();

		static AudioSubsystem				audioSubsystem;
		static GameStateManager				stateMan;
		static InputManager					inputMan;
		static CommandLineArgs				commandLine;

		static GenericContainer<RSC_Sprite>		rscSpriteMan;
		static GenericContainer<RSC_Sound>		rscSoundMan;
		static GenericContainer<RSC_Music>		rscMusicMan;
		static GenericContainer<RSC_Script>		rscScriptMan;
		static GenericContainer<RSC_Texture>	rscTexMan;
		static GenericContainer<RSC_Map>		rscMapMan;
		static GenericContainer<RSC_Font>		rscFontMan;

		//RSC_GLPrograms don't own shaders, but obviously make extensive use of them
		//be careful not to delete shaders in use by programs
		static GenericContainer<RSC_GLShader>	rscShaderMan;
		static GenericContainer<RSC_GLProgram>	rscShaderProgramMan;

		static ImGuiState guiState;

		static bool DEBUG_MODE();

		static void ImGuiCreateFontsTexture();
		static void ImGuiInvalidateFontTexture();
	private:
		static bool debugMode;

		Kernel();
		~Kernel();

		///Is called at the start of every new frame for the sake of imgGui
		static void ImGuiNewFrame(SDL_Window* window);
		///Is called once per run to initialize imgui resources
		static void ImGuiCreateDeviceObjects();

		static int gameLoops;
		static unsigned int nextGameTick;

		static int returnValue;
		static SDLInit* SDLMan;

};

//for more laconic access
#define K_StateMan		Kernel::stateMan
#define K_CommandLine	Kernel::commandLine
#define K_AudioSub		Kernel::audioSubsystem

#define K_TextureMan	Kernel::rscTexMan
#define K_SpriteMan		Kernel::rscSpriteMan
#define K_SoundMan		Kernel::rscSoundMan
#define K_MusicMan		Kernel::rscMusicMan
#define K_ScriptMan		Kernel::rscScriptMan
#define K_MapMan		Kernel::rscMapMan
#define K_FontMan		Kernel::rscFontMan
#define K_ShaderMan		Kernel::rscShaderMan
#define K_ShaderProgramMan		Kernel::rscShaderProgramMan

#endif
