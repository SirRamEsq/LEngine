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

#include "AudioSubsystem.h"
#include "CommandLineArgs.h"

#include <SOIL/SOIL.h>

#include <vector>

class Kernel{
	public:
		static void Inst(int argc, char *argv[]);
		static void Inst();
		static bool Run();
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

		//RSC_GLPrograms don't own shaders, but obviously make extensive use of them
		//be careful not to delete shaders in use by programs
		static GenericContainer<RSC_GLShader>	rscShaderMan;
		static GenericContainer<RSC_GLProgram>	rscShaderProgramMan;

	private:
		Kernel();
		~Kernel();

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
#define K_ShaderMan		Kernel::rscShaderMan
#define K_ShaderProgramMan		Kernel::rscShaderProgramMan

#endif
