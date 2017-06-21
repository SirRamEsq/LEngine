#include "Kernel.h"
#ifdef TEST_Kernel_MOCK

Kernel*                     Kernel::pointertoself;
EventDispatcher             Kernel::eventMan(NULL, NULL);
SDLInit*                    Kernel::SDLMan;
GameStateManager            Kernel::stateMan;
EntityManager*              Kernel::entMan;
/*
InputManager                Kernel::inputMan;
CommandLineArgs             Kernel::commandLine;
AudioSubsystem              Kernel::audioSubsystem;
*/
int          Kernel::gameLoops;
unsigned int Kernel::nextGameTick;
int          Kernel::returnValue;

GenericContainer<LSprite>      Kernel::rscSpriteMan;
GenericContainer<LTexture>     Kernel::rscTexMan;
GenericContainer<L_GL_Shader>  Kernel::rscShaderMan;
GenericContainer<LSound>       Kernel::rscSoundMan;
GenericContainer<LMusic>       Kernel::rscMusicMan;
GenericContainer<LScript>      Kernel::rscScriptMan;
GenericContainer<LMap>         Kernel::rscMapMan;

Kernel::Kernel(){}
Kernel::~Kernel(){}

void Kernel::Close(){
    ErrorLog::WriteToFile("Closing...", ErrorLog::GenericLogFile);

    //stateMan.Close();
    ErrorLog::CloseFiles();
    SDLMan->CloseSDL();
    rscTexMan.Clear();

    delete entMan;
}

Kernel* Kernel::Instance(){
    if(pointertoself==NULL){pointertoself=new Kernel();}
    return pointertoself;
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

    rscTexMan       .SetLoadFunction(&LTexture::LoadResource);
    stateMan.Init();
    entMan      = new EntityManager;
    rscSpriteMan    .SetLoadFunction(&LSprite::LoadResource);
    rscMusicMan     .SetLoadFunction(&LMusic::LoadResource   );
    rscSoundMan     .SetLoadFunction(&LSound::LoadResource   );
    rscScriptMan    .SetLoadFunction(&LScript::LoadResource);
//    rscMapMan       .SetLoadFunction(&LoadMAP   );

/*
    commandLine.ParseArgs(argc, argv);


    stateMan.PushState(std::move( std::unique_ptr<GameStartState> (new GameStartState(&stateMan)) ));
    stateMan.UpdateCurrentState();
*/
    gameLoops=0;
    //nextGameTick=SDL_GetTicks() - 1;
}

bool Kernel::Run(){

}

#endif
