#include "StateManager.h"
#include "Kernel.h"


GameState::GameState(GameStateManager* gsm)
    :gameStateManager(gsm), luaInterface(this, SCREEN_W, SCREEN_H, CAMERA_W, CAMERA_H),
    eventDispatcher(),
    comPosMan(&eventDispatcher),
    comCameraMan(&eventDispatcher),
    comSpriteMan(&eventDispatcher),
    comInputMan(&eventDispatcher),

    comCollisionMan(&eventDispatcher),
    comParticleMan(&eventDispatcher),
    comScriptMan(luaInterface.GetState(), &luaInterface, &eventDispatcher),
	comLightMan(&eventDispatcher){

    mCurrentMap=NULL;
    SetDependencies();
}

void GameState::SetDependencies(){
    input = gameStateManager->input.SetEventDispatcher(&eventDispatcher);

    entityMan.RegisterComponentManager(&comPosMan,          EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
    entityMan.RegisterComponentManager(&comInputMan,        EntityManager::DEFAULT_UPDATE_ORDER::INPUT);
    entityMan.RegisterComponentManager(&comScriptMan,       EntityManager::DEFAULT_UPDATE_ORDER::SCRIPT);
    entityMan.RegisterComponentManager(&comSpriteMan,       EntityManager::DEFAULT_UPDATE_ORDER::SPRITE);
    entityMan.RegisterComponentManager(&comCollisionMan,    EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);
    entityMan.RegisterComponentManager(&comParticleMan,     EntityManager::DEFAULT_UPDATE_ORDER::PARTICLE);
    entityMan.RegisterComponentManager(&comCameraMan,       EntityManager::DEFAULT_UPDATE_ORDER::CAMERA);
    entityMan.RegisterComponentManager(&comLightMan,        EntityManager::DEFAULT_UPDATE_ORDER::LIGHT);
	comInputMan.SetDependency(input);
	eventDispatcher.SetDependencies(gameStateManager, &entityMan);
    comCollisionMan.SetDependencies(&comPosMan);
	comCameraMan.SetDependencies(&comPosMan, &renderMan);
    comInputMan.SetDependency(input);
	comScriptMan.SetDependencies(&renderMan);
	comParticleMan.SetDependencies(&renderMan, &comPosMan);
}

void GameState::DrawPreviousState(){
    gameStateManager->DrawPreviousState(this);
}
void GameState::UpdateComponentManagers(){
    //Game Logic
        //Should consolidate input man's functionality into script man? doesn't really do much...
        comInputMan.     Update(); //Get Input

        //could use concurrency within particleMan
        comParticleMan.  Update(); //Update particles

        //should order be pos+col then script? or script then pos+col?
        //I think pos+col then script, as this way scripts can inquire as to each others (correct) position every frame

        comPosMan.       Update(); //Use script from previous frame to update position+
        comCollisionMan. Update(); //Use newly generated position in collision (and change position if needed)
		//Use new position to set camera values
		comCameraMan.Update();
        comScriptMan.    Update(); //Use input to run script and set next frame coords

        //Could conceivably use concurrency here; generate all collision events using multiple threads, then process with a single thread


    //Render
        //Each of these could be put on their own thread and run concurrently comCameraMan.    Update(); //Update cameras
        comLightMan.     Update(); //Update lights and generate light map
        comSpriteMan.    Update(); //Update sprites

    //execution now jumps to rendermanager
}

GameStateManager::GameStateManager(){
   	mCurrentState=NULL;
}

void GameStateManager::Close(){
    while(mGameStates.empty()==false){
        mGameStates.back()->Close();
        mGameStates.pop_back();
    }
 	mCurrentState=NULL;
}

void GameStateManager::PushState(std::unique_ptr<GameState> state){
    mGameStates.push_back( std::move(state) );
    mCurrentState=mGameStates.back().get();
    mCurrentState->Init();
}

void GameStateManager::UpdateCurrentState(){
    mCurrentState=mGameStates.back().get();
}

void GameStateManager::PopState(){
    if(mGameStates.empty()){
        return;
    }
    //Inform the state that it is being deleted
    mGameStates.back()->Close();

    //Delete state
    mGameStates.pop_back();

    if(mGameStates.empty()){
        mCurrentState=NULL;
        return;
    }

    mCurrentState  = mGameStates.back().get();
    input.SetEventDispatcher(&mCurrentState->eventDispatcher, &mCurrentState->input);
}

void GameStateManager::DrawPreviousState(GameState* gs){
    GameState* prev=GetPreviousState(gs);
    if(prev!=NULL){
        prev->Draw();
    }
}

GameState* GameStateManager::GetPreviousState(GameState* gs){
    //reverse iterator, as this function will most likely be called with a pointer to the back of the vector
    for( auto i = mGameStates.rbegin(); i != mGameStates.rend(); i++){
        if(gs==(i->get())){
            return (i->get());
        }
    }
    return NULL;
}

void GameStateManager::HandleEvent(const Event* event){
    if(mCurrentState!=NULL){mCurrentState->HandleEvent(event);}
}

bool GameStateManager::Update(){
    mCurrentState->entityMan.Cleanup();
    input.HandleInput();

    if(mCurrentState!=NULL){return mCurrentState->Update();}

    return false;
}

void GameStateManager::Draw(){
    if(mCurrentState!=NULL){mCurrentState->Draw();}
}

void GameState::SetMapHandleRenderableLayers(const std::map<MAP_DEPTH, TiledLayerGeneric*>& layers){
    for(auto i = layers.begin(); i != layers.end(); i++){
        if(i->second->layerType == LAYER_TILE){
            auto layer = make_unique<RenderTileLayer> (&renderMan, (TiledTileLayer*)i->second);
            mCurrentMapTileLayers.push_back( std::move(layer) );
        }

        if(i->second->layerType == LAYER_IMAGE){
            auto layer = make_unique<RenderImageLayer> (&renderMan, (TiledImageLayer*)i->second);
            mCurrentMapTileLayers.push_back( std::move(layer) );
        }
    }
}


std::vector<EID> GameState::SetMapGetEntitiesUsingEntrances(const std::vector<std::unique_ptr<TiledObjectLayer> >& layers){
    std::vector<EID> objectsUsingEntrance;

    for(auto ii = layers.begin(); ii != layers.end(); ii++){
        for(auto objectIt=(*ii)->objects.begin(); objectIt!=(*ii)->objects.end(); objectIt++){

            //Include object if it utilizes an entrance
            if((objectIt)->second.useEntrance){
                EID id = entityMan.NewEntity(objectIt->second.name);
                objectsUsingEntrance.push_back(id);
            }

        }
    }

    return objectsUsingEntrance;
}

//returns a data structure mapping tiled EIDS to engine EIDS
std::map<EID,EID> GameState::SetMapCreateEntitiesFromLayers(const std::vector<std::unique_ptr<TiledObjectLayer> >& layers){

    std::map<EID,EID> tiledIDtoEntityID; //This is for the purpose of linking event listeners later

    for(auto ii = layers.begin(); ii != layers.end(); ii++){
        for(auto objectIt=(*ii)->objects.begin(); objectIt!=(*ii)->objects.end(); objectIt++){
            //New Entity
            //If the entity has a name, map that name to it's engine generated ID
            EID ent = entityMan.NewEntity(objectIt->second.name);

            //Map Tile map ID to Engine generated ID
            tiledIDtoEntityID[objectIt->first]=ent;

            //Set position
            comPosMan.AddComponent(ent);
            ((ComponentPosition*)(comPosMan.GetComponent(ent)) )->SetPositionLocal( Coord2df(objectIt->second.x, objectIt->second.y) );

            //Add script Component and set to be initialized later during linking stage after all entities have EIDs
            if(objectIt->second.script!=""){
                comScriptMan.AddComponent(ent);
            }

            //Set up Light source (if applicable)
            if(objectIt->second.light==true){
                comLightMan.AddComponent(ent);
            }

            //Special treatment to certain object types
            else if(objectIt->second.type==global_TiledStrings[TILED_CAMERA]){
                if((objectIt->second.flags & TILED_OBJECT_IS_MAIN_CAMERA)==TILED_OBJECT_IS_MAIN_CAMERA){
                    comCameraMan.AddComponent(ent);
                    ComponentCamera* cam=((ComponentCamera*)(comCameraMan.GetComponent(ent)));
                    cam->SetAsMainCamera();
                }
            }
        }
    }

    return tiledIDtoEntityID;
}

void GameState::SetMapLinkEntities(
    const std::vector<std::unique_ptr<TiledObjectLayer> >& layers,
    const std::map<EID,EID>& tiledIDtoEntityID,
    const std::vector<EID>& objectsUsingEntrance
){

    MapEntrance* correctEntrance=NULL;
    int correctEntranceX=0;
    int correctEntranceY=0;

    for(auto ii = layers.begin(); ii != layers.end(); ii++){
        //second pass (Linking)
        EID entityID,listenerID;
        EID parent=0;
        EID parentEID=0;
        EID child=0;
        for(auto objectIt=(*ii)->objects.begin(); objectIt!=(*ii)->objects.end(); objectIt++){

            parent      =   objectIt->second.parent;
            child       =   tiledIDtoEntityID.find(objectIt->first)->second;
            parentEID   =   tiledIDtoEntityID.find(parent)->second;

            //Initialize Script
            std::string& scriptName = objectIt->second.script;

            if(scriptName!=""){
                const LScript* script=K_ScriptMan.GetItem(scriptName);
                if(script==NULL){
                    K_ScriptMan.LoadItem(scriptName,scriptName);
                    script=K_ScriptMan.GetItem(scriptName);
                    if(script==NULL){
                        ErrorLog::WriteToFile("StateManager::SetCurrentMap; Couldn't Load Script Named: " + objectIt->second.script, ErrorLog::GenericLogFile);
                    }
                }
                if(script!=NULL){
                    //entities have been created so that parents can be assigned properly
                    luaInterface.RunScript(child, script, (*ii)->GetDepth(), parentEID, objectIt->second.name, objectIt->second.type, &objectIt->second, NULL);
                }
            }


            //Event Linking
            //Can move into scriptManager
            for(auto eventSource=objectIt->second.eventSources.begin(); eventSource!=objectIt->second.eventSources.end(); eventSource++){
                entityID    = tiledIDtoEntityID.find((*eventSource))->second;
                listenerID  = tiledIDtoEntityID.find(objectIt->first)->second;

                ComponentScript* listenerScript=((ComponentScript*)comScriptMan.GetComponent(listenerID));
                ComponentScript* senderScript=((ComponentScript*)comScriptMan.GetComponent(entityID));
                if((listenerScript==NULL)or(senderScript==NULL)){
                    std::stringstream ss;
                    ss << "[Map Loader: " << mCurrentMap->GetMapName() << "]" << " Couldn't listen to event from TiledID " << *eventSource
                        << "     Listener TiledID is: " << objectIt->first;
                    ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
                    continue;
                }
                senderScript->EventLuaAddObserver(listenerScript);
            }


            //Parent Linking
            if(parent!=0){
                ComponentScript* scriptComp =((ComponentScript*)  (comScriptMan.GetComponent(child)));

                //if entity has a script, it's up to the script to decide how to handle the parent, else it's assigned to all components
                if(scriptComp==NULL){
                    ((ComponentPosition*)  (comPosMan.GetComponent(child)))->ChangeParent(parentEID);
                }
            }
        }
        /*
        //Link the map script as well
        for(auto eventSource=mCurrentMap->mEventSources.begin(); eventSource!=mCurrentMap->mEventSources.begin(); eventSource++){
            entityID = tiledIDtoEntityID.find((*eventSource))->second;

            ComponentScript* listenerScript=((ComponentScript*)comScriptMan.GetComponent(listenerID));
            if(listenerScript==NULL){
                std::stringstream ss;
                ss << "[Map Loader: " << mCurrentMap->GetMapName() << "]" << " Couldn't listen to event from TiledID ";
                ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
                continue;
            }

            mCurrentMap->mScriptComp->EventLuaAddObserver(listenerScript);
        }*/
    }

    //Set all objects that use entrances to their proper entrance
    if(correctEntrance!=NULL){
        for(auto i = objectsUsingEntrance.begin(); i != objectsUsingEntrance.end(); i++){
            ((ComponentPosition*)(comPosMan.GetComponent(*i)) )->SetPositionLocal( Coord2df(correctEntranceX, correctEntranceY) );
        }
    }
}


bool GameState::SetCurrentMap(const I_RSC_Map* m, unsigned int entranceID){
    if(m==NULL){
        ErrorLog::WriteToFile("Error: GameState::SetCurrentTiledMap was passed a NULL Pointer", ErrorLog::GenericLogFile);
        return false;
    }

    //Unload all layers from last map
    mCurrentMapTileLayers.clear();
	//Clear all entites from current state
    entityMan.ClearAllEntities();
	//Actually delete all entities
	entityMan.Cleanup();

    //Copy map passed
    mCurrentMap.reset();
    mCurrentMap = make_unique<RSC_MapImpl>(*((RSC_MapImpl*)(m)));

    TiledData* td=(mCurrentMap->GetTiledData());
    SetMapHandleRenderableLayers(td->tiledRenderableLayers);

    const std::vector<std::unique_ptr<TiledObjectLayer> >& layers = td->tiledObjectLayers;
    auto tiledIDtoEntityID = SetMapCreateEntitiesFromLayers(layers);
    auto objectsUsingEntrance = SetMapGetEntitiesUsingEntrances(layers);

    SetMapLinkEntities(layers, tiledIDtoEntityID, objectsUsingEntrance);

    //Try to run this map's script
    /*if(mCurrentMap->GetGlobalScriptName()!=""){
        //mCurrentMap->mScriptComp=new ComponentScript(EID_MAPSCRIPT, K_ComScriptMan->logFileName);
        if(!mCurrentMap->mScriptComp->FullInitialize(mCurrentMap->GetGlobalScriptName(), 0, 0, "MAPSCRIPT", "MAPSCRIPT")){
            delete mCurrentMap->mScriptComp;
            ErrorLog::WriteToFile("Couldn't load map script", ErrorLog::GenericLogFile);
            ErrorLog::WriteToFile(mCurrentMap->GetGlobalScriptName(), ErrorLog::GenericLogFile);
        }
    }
    else{mCurrentMap->mScriptComp=NULL;}*/

    return true;
}
