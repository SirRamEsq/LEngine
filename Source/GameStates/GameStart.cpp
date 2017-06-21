#include "GameStart.h"
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
const LMap* changeMapM=NULL;
std::string changeMapS="";
int changeMapEntrance=0;
int DEBUGrepeat=0;

void GameStartState::Init(){
    countdown=1;
    pause=false;

    CommandLineArgs& cmd=(K_CommandLine);
    std::string mapName=cmd.GetValue(L_CMD_LEVELNAME);

    std::unique_ptr<const LMap> mapData;
    if(mapName==""){
        mapName="mmap2.tmx";
    }
    mapData=LoadMAP(mapName);

    if(mapData==NULL){
        ErrorLog::WriteToFile("Couldn't load map named: " + mapName, ErrorLog::GenericLogFile);
        countdown=0;
    }
    K_MapMan.LoadItem(mapName,  mapData);

    SetCurrentMap(K_MapMan.GetItem(mapName), 2);

    t= new RenderText(0,0,"FR", false);
    t->SetColorI(0,100,50);
    t->SetDepth(-1000);

    ErrorLog::WriteToFile("Done Loading", ErrorLog::GenericLogFile);

    timeCount=SDL_GetTicks();
}

void GameStartState::Close(){
}

void GameStartState::HandleEvent(const Event* event){
    if(event->message==MSG_KEYDOWN){
        if     ( *((std::string*)event->extradata) == "use")    {countdown=-1;}
        else if( *((std::string*)event->extradata) == "pause")  {gameStateManager->PushState(std::unique_ptr<GamePauseState> (new GamePauseState(gameStateManager)));}

        else if( *((std::string*)event->extradata) == "up")      {u=true;}
        else if( *((std::string*)event->extradata) == "down")    {d=true;}
        else if( *((std::string*)event->extradata) == "left")    {l=true;}
        else if( *((std::string*)event->extradata) == "right")   {r=true;}
    }

    if(event->message==MSG_KEYUP){
        if     ( *((std::string*)event->extradata) == "up")      {u=false;}
        else if( *((std::string*)event->extradata) == "down")    {d=false;}
        else if( *((std::string*)event->extradata) == "left")    {l=false;}
        else if( *((std::string*)event->extradata) == "right")   {r=false;}
    }

    /*if(event->message==MSG_MAP_CHANGE){
        EColPacket* packet=((EColPacket*)(event->extradata));
        std::string mapName=((MapExit*)(packet->extraData))->mMapName;
        changeMapEntrance=((MapExit*)(packet->extraData))->mEntranceID;
        const LMap* newMap=K_MapMan.GetItem(mapName);
        if(newMap==NULL){
            std::unique_ptr<const LMap> loadNewMap (LoadMAP(mapName));
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

bool GameStartState::Update(){
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

void GameStartState::Draw(){
    renderMan.Render();
}
