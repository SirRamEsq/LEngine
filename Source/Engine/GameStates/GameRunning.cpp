#include "GameRunning.h"
#include "../Kernel.h"
#include "../RenderManager.h"

RenderText *t=NULL;
int y=0;
int xx=0;
bool yrev=false;
int movspd=2;
bool u=false;
bool r=false;
bool d=false;
bool l=false;

long int frameCount=0;
long int timeCount=0;

bool changeMap=false;
const RSC_Map* changeMapM=NULL;
std::string changeMapS="";
int changeMapEntrance=0;
int DEBUGrepeat=0;

GameRunningState::GameRunningState(GameStateManager* gsm)
	: GameState(gsm){

}

GameRunningState::~GameRunningState(){

}

void GameRunningState::Init(const RSC_Script* stateScript){
    countdown=1;
    pause=false;

    CommandLineArgs& cmd=(K_CommandLine);
    std::string mapName=cmd.GetValue(L_CMD_LEVELNAME);

    std::unique_ptr<const RSC_Map> mapData;
    if(mapName==""){
        mapName="MAP1.tmx";
    }
    mapData=RSC_MapImpl::LoadResource(mapName);

    if(mapData==NULL){
        ErrorLog::WriteToFile("Couldn't load map named: " + mapName, ErrorLog::GenericLogFile);
        countdown=0;
    }
    K_MapMan.LoadItem(mapName,  mapData);

    SetCurrentMap(K_MapMan.GetItem(mapName), 2);

    t= new RenderText(&renderMan, 0,0,"FR", false);
    t->SetColorI(0,100,50);
    t->SetDepth(-1000);

    ErrorLog::WriteToFile("Done Loading", ErrorLog::GenericLogFile);

    timeCount=SDL_GetTicks();
}

void GameRunningState::Close(){
}

void GameRunningState::Resume(){

}

void GameRunningState::HandleEvent(const Event* event){
    if(event->message == Event::MSG::KEYDOWN){
		std::string inputKey = event->description;

        if     ( inputKey == "use")    {countdown=-1;}
        else if( inputKey == "pause")  {gameStateManager->PushState(make_unique<GamePauseState> (gameStateManager));}

        else if( inputKey == "up")      {u=true;}
        else if( inputKey == "down")    {d=true;}
        else if( inputKey == "left")    {l=true;}
        else if( inputKey == "right")   {r=true;}
    }

    if(event->message == Event::MSG::KEYUP){
		std::string inputKey = event->description;

        if     ( inputKey == "up")      {u=false;}
        else if( inputKey == "down")    {d=false;}
        else if( inputKey == "left")    {l=false;}
        else if( inputKey == "right")   {r=false;}
    }

    /*if(event->message==MSG_MAP_CHANGE){
        EColPacket* packet=((EColPacket*)(event->extradata));
        std::string mapName=((MapExit*)(packet->extraData))->mMapName;
        changeMapEntrance=((MapExit*)(packet->extraData))->mEntranceID;
        const RSC_Map* newMap=K_MapMan.GetItem(mapName);
        if(newMap==NULL){
            std::unique_ptr<const RSC_Map> loadNewMap (LoadMAP(mapName));
            if(loadNewMap.get()==NULL){
                ErrorLog::WriteToFile("Bad Map Name Passed", ErrorLog::GenericLogFile);
                ErrorLog::WriteToFile(mapName, ErrorLog::GenericLogFile);
                return;
            }
            newMap=loadNewMap.get();
            K_MapMan.LoadItem(mapName,  loadNewMap);
        }
        DEBUGrepeat+=1;
        //ErrorLog::WriteToFile("Setting to true; DEBUGREPEAT= ", DEBUGrepeat);

        changeMap=true;
        changeMapM=newMap;
        changeMapS=mapName;
        //Set all the entities to be cleared at the end of this frame
        K_EntMan->ClearAllEntities();
        K_InputMan.ClearAllListeners();
    }*/
}

bool GameRunningState::Update(){
    if(countdown<0){return false;}

    if(changeMap){
        DEBUGrepeat=0;
        changeMap=false;
        SetCurrentMap(K_MapMan.GetItem(changeMapS), changeMapEntrance);
    }

    //GetCurrentMap()->tiledData.AnimateAgain();
    UpdateComponentManagers();

    SDL_Color c=t->GetColor();

    c.r+=4;
    c.b+=2;
    c.g+=3;

    t->SetColor(c);
    frameCount+=1;

    if((SDL_GetTicks() - timeCount) >= 1000){
        t->ChangeText(SSTR(frameCount));
        timeCount=SDL_GetTicks();
        frameCount=0;
    }
    return true;
}

void GameRunningState::Draw(){
    renderMan.Render();
}
