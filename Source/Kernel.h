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
#include "Resources/LTexture.h"
#include "Resources/LSprite.h"
#include "Resources/LSound.h"
#include "Resources/LScript.h"
#include "Resources/LHeightmap.h"
#include "Resources/LBackground.h"
#include "Resources/RSC_Map.h"
#include "Resources/LglShader.h"

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

        static AudioSubsystem               audioSubsystem;
        static GameStateManager             stateMan;
        static InputManager                 inputMan;
        static CommandLineArgs              commandLine;

        static GenericContainer<LSprite>       rscSpriteMan;
        static GenericContainer<LSound>        rscSoundMan;
        static GenericContainer<LMusic>        rscMusicMan;
        static GenericContainer<LScript>       rscScriptMan;
        static GenericContainer<LTexture>      rscTexMan;
        static GenericContainer<I_RSC_Map>     rscMapMan;
        static GenericContainer<L_GL_Shader>   rscShaderMan;

    private:
        Kernel();
        ~Kernel();

        static int gameLoops;
        static unsigned int nextGameTick;

        static int returnValue;
        static SDLInit* SDLMan;
};

//for more laconic access
#define K_StateMan      Kernel::stateMan
#define K_CommandLine   Kernel::commandLine
#define K_AudioSub      Kernel::audioSubsystem

#define K_TextureMan    Kernel::rscTexMan
#define K_SpriteMan     Kernel::rscSpriteMan
#define K_SoundMan      Kernel::rscSoundMan
#define K_MusicMan      Kernel::rscMusicMan
#define K_ScriptMan     Kernel::rscScriptMan
#define K_MapMan        Kernel::rscMapMan
#define K_ShaderMan     Kernel::rscShaderMan

#endif
