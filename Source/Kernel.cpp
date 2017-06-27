#include "Kernel.h"

#ifndef TEST_Kernel_MOCK

SDLInit*                    Kernel::SDLMan;
GameStateManager            Kernel::stateMan;
InputManager                Kernel::inputMan;
CommandLineArgs             Kernel::commandLine;
AudioSubsystem              Kernel::audioSubsystem;

int          Kernel::gameLoops;
unsigned int Kernel::nextGameTick;
int          Kernel::returnValue;

GenericContainer<LSprite>      Kernel::rscSpriteMan;
GenericContainer<LTexture>     Kernel::rscTexMan;
GenericContainer<LSound>       Kernel::rscSoundMan;
GenericContainer<LMusic>       Kernel::rscMusicMan;
GenericContainer<LScript>      Kernel::rscScriptMan;
GenericContainer<I_RSC_Map>    Kernel::rscMapMan;
GenericContainer<L_GL_Shader>  Kernel::rscShaderMan;

Kernel::Kernel(){}
Kernel::~Kernel(){}

void Kernel::Close(){
    ErrorLog::WriteToFile("Closing...", ErrorLog::GenericLogFile);

    stateMan.Close();
    ErrorLog::CloseFiles();
    SDLMan->CloseSDL();

    rscTexMan    .Clear();
    rscSpriteMan .Clear();
    rscMusicMan  .Clear();
    rscSoundMan  .Clear();
    rscScriptMan .Clear();
    rscMapMan    .Clear();
}

void Kernel::Inst(){
    int argc=0; char *argv[0];
    Kernel::Inst(argc, argv);
}
void Kernel::Inst(int argc, char *argv[]){
    ErrorLog::Inst();
    ErrorLog::OpenFile(ErrorLog::GenericLogFile);
    ErrorLog::WriteToFile("Starting up...", ErrorLog::GenericLogFile);

    PHYSFS_init(NULL);
    std::string searchPath="Data/";
    PHYSFS_addToSearchPath(searchPath.c_str(), 0);

    SDLMan=SDLInit::Inst();
    SDLMan->InitSDL();
    SDLMan->InitOpenGL();

    rscTexMan       .SetLoadFunction(&LTexture::LoadResource   );
    rscSpriteMan    .SetLoadFunction(&LSprite::LoadResource   );
    rscMusicMan     .SetLoadFunction(&LMusic::LoadResource   );
    rscSoundMan     .SetLoadFunction(&LSound::LoadResource   );
    rscScriptMan    .SetLoadFunction(&LScript::LoadResource);
    rscMapMan       .SetLoadFunction(&RSC_MapImpl::LoadResource   );

    stateMan.Init();
    commandLine.ParseArgs(argc, argv);

    gameLoops=0;
    nextGameTick=SDL_GetTicks() - 1;
}

bool Kernel::Run(){
    gameLoops=0;

    //loop seems to be locked to 60fps no matter what?
    while(SDL_GetTicks()>nextGameTick) {
        nextGameTick = SDL_GetTicks() + SKIP_TICKS;


        returnValue=stateMan.Update();
        if(returnValue!=1){
            stateMan.PopState();
            if(stateMan.IsEmpty()){
                return returnValue;
            }
        }
        stateMan.UpdateCurrentState();

        //Audio subsystem can be put on a different thread
        audioSubsystem.ProcessEvents();

        gameLoops++;

        //If we're behind, skip drawing
        //Don't skip if the max amount of frame skip has been passed
        if( (SDL_GetTicks()<nextGameTick) or (gameLoops>MAX_FRAMESKIP) ){
            //game render
            stateMan.Draw();
            glFinish();
        }
        SDL_GL_SwapWindow(SDLMan->GetWindow());
    }
    return true;
}

#endif
