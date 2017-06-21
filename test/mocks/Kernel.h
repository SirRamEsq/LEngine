#ifndef L_ENGINE_KERNEL_MOCK
#define L_ENGINE_KERNEL_MOCK

#include "../../Source/Errorlog.h"
#include "../../Source/GenericContainer.h"
#include "physfs.h"
#include "Event.h"
#include "../../Source/StateManager.h"

#include "../../Source/EntityManager.h"
#include "../../Source/Resources/ResourceLoading.h"
#include "../../Source/Resources/LTexture.h"
#include "../../Source/Resources/LSprite.h"
/*
#include "Resources/LSound.h"
#include "Resources/LScript.h"
#include "Resources/LObj.h"
#include "Resources/LTDF.h"
#include "Resources/LHeightmap.h"
#include "Resources/LBackground.h"
#include "Resources/LEvent.h"
#include "Resources/LMap.h"
#include "Resources/LglShader.h"

#include "AudioSubsystem.h"
#include "CommandLineArgs.h"*/

#include <SOIL/SOIL.h>

#include <vector>

class Kernel{
    public:
        static void Inst(int argc, char *argv[]);
        static void Inst();
        static bool Run();
        static void Close();

        static EventDispatcher              eventMan;
        static GameStateManager             stateMan;
        /*static AudioSubsystem               audioSubsystem;
        static InputManager                 inputMan;
        static CommandLineArgs              commandLine;*/

        //Need moved to stateMan
        static EntityManager*               entMan;


        static GenericContainer<LTexture>      rscTexMan;
        static GenericContainer<LSprite>       rscSpriteMan;
        static GenericContainer<LSound>        rscSoundMan;
        static GenericContainer<LMusic>        rscMusicMan;
        static GenericContainer<LScript>       rscScriptMan;
        static GenericContainer<LMap>          rscMapMan;
        static GenericContainer<L_GL_Shader>   rscShaderMan;

        static Kernel* pointertoself;
        static Kernel* Instance();

    private:
        Kernel();
        ~Kernel();

        static int gameLoops;
        static unsigned int nextGameTick;

        static int returnValue;
        static SDLInit* SDLMan;
};

//for more laconic access
#define K_Kern          Kernel::Instance()
#define K_EntMan        Kernel::Instance()->entMan
#define K_StateMan      Kernel::Instance()->stateMan
#define K_EventMan      Kernel::Instance()->eventMan
#define K_InputMan      Kernel::Instance()->inputMan
#define K_CommandLine   Kernel::Instance()->commandLine
#define K_AudioSub      Kernel::Instance()->audioSubsystem

#define K_TextureMan    Kernel::Instance()->rscTexMan
#define K_SpriteMan     Kernel::Instance()->rscSpriteMan
#define K_SoundMan      Kernel::Instance()->rscSoundMan
#define K_MusicMan      Kernel::Instance()->rscMusicMan
#define K_ScriptMan     Kernel::Instance()->rscScriptMan
#define K_MapMan        Kernel::Instance()->rscMapMan
#define K_ShaderMan     Kernel::Instance()->rscShaderMan

#endif // L_ENGINE_KERNEL_MOCK
