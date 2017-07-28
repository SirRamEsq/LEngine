#include "Kernel.h"

#ifndef TEST_Kernel_MOCK

SDLInit*					Kernel::SDLMan;
GameStateManager			Kernel::stateMan;
InputManager				Kernel::inputMan;
CommandLineArgs				Kernel::commandLine;
AudioSubsystem				Kernel::audioSubsystem;

int			 Kernel::gameLoops;
unsigned int Kernel::nextGameTick;
int			 Kernel::returnValue;

GenericContainer<RSC_Sprite>	Kernel::rscSpriteMan;
GenericContainer<RSC_Texture>	Kernel::rscTexMan;
GenericContainer<RSC_Sound>		Kernel::rscSoundMan;
GenericContainer<RSC_Music>		Kernel::rscMusicMan;
GenericContainer<RSC_Script>	Kernel::rscScriptMan;
GenericContainer<RSC_Map>		Kernel::rscMapMan;
GenericContainer<RSC_GLShader>	Kernel::rscShaderMan;
GenericContainer<RSC_GLProgram>	Kernel::rscShaderProgramMan;

Kernel::Kernel(){}
Kernel::~Kernel(){}

void Kernel::Close(){
	ErrorLog::WriteToFile("Closing...", ErrorLog::GenericLogFile);

	stateMan.Close();
	ErrorLog::CloseFiles();
	SDLMan->CloseSDL();

	rscTexMan	 .Clear();
	rscSpriteMan .Clear();
	rscMusicMan  .Clear();
	rscSoundMan  .Clear();
	rscScriptMan .Clear();
	rscMapMan	 .Clear();
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

	rscTexMan		.SetLoadFunction(&RSC_Texture::LoadResource   );
	rscSpriteMan	.SetLoadFunction(&RSC_Sprite::LoadResource	 );
	rscMusicMan		.SetLoadFunction(&RSC_Music::LoadResource	);
	rscSoundMan		.SetLoadFunction(&RSC_Sound::LoadResource	);
	rscScriptMan	.SetLoadFunction(&RSC_Script::LoadResource);
	rscMapMan		.SetLoadFunction(&RSC_MapImpl::LoadResource   );

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
