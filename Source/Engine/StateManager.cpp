#include "StateManager.h"
#include "Kernel.h"
#include "TiledProperties.h"

GameState::GameState(GameStateManager *gsm)
    : gameStateManager(gsm),
      luaInterface(this),
      eventDispatcher(),
      comPosMan(&eventDispatcher),
      comCameraMan(&eventDispatcher),
      comSpriteMan(&eventDispatcher),
      comInputMan(&eventDispatcher),

      comCollisionMan(&eventDispatcher),
      comParticleMan(&eventDispatcher),
      comScriptMan(luaInterface.GetState(), &luaInterface, &eventDispatcher),
      comLightMan(&eventDispatcher),

      entityMan(gsm) {
  mCurrentMap = NULL;
  SetDependencies();

  nextMap = NULL;
  nextMapEntrance = 0;
  isLuaState = false;
}

GameState::~GameState() {}

void GameState::SetDependencies() {
  input = gameStateManager->inputManager->SetEventDispatcher(&eventDispatcher);

  entityMan.RegisterComponentManager(
      &comPosMan, EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
  entityMan.RegisterComponentManager(
      &comInputMan, EntityManager::DEFAULT_UPDATE_ORDER::INPUT);
  entityMan.RegisterComponentManager(
      &comScriptMan, EntityManager::DEFAULT_UPDATE_ORDER::SCRIPT);
  entityMan.RegisterComponentManager(
      &comSpriteMan, EntityManager::DEFAULT_UPDATE_ORDER::SPRITE);
  entityMan.RegisterComponentManager(
      &comCollisionMan, EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);
  entityMan.RegisterComponentManager(
      &comParticleMan, EntityManager::DEFAULT_UPDATE_ORDER::PARTICLE);
  entityMan.RegisterComponentManager(
      &comCameraMan, EntityManager::DEFAULT_UPDATE_ORDER::CAMERA);
  entityMan.RegisterComponentManager(
      &comLightMan, EntityManager::DEFAULT_UPDATE_ORDER::LIGHT);
  comInputMan.SetDependency(input);
  eventDispatcher.SetDependencies(gameStateManager, &entityMan);
  comCollisionMan.SetDependencies(&comPosMan);
  comSpriteMan.SetDependencies(&comPosMan, &renderMan);
  comCameraMan.SetDependencies(&comPosMan, &renderMan);
  comInputMan.SetDependency(input);
  comScriptMan.SetDependencies(&renderMan);
  comParticleMan.SetDependencies(&renderMan, &comPosMan);
}

void GameState::DrawPreviousState() {
  gameStateManager->DrawPreviousState(this);
}
void GameState::UpdateComponentManagers() {
  CreateNewEntities();
  luaInterface.Update();

  // Game Logic
  // Should consolidate input man's functionality into script man? doesn't
  // really do much...
  comInputMan.Update();  // Get Input

  // could use concurrency within particleMan
  comParticleMan.Update();  // Update particles

  // should order be pos+col then script? or script then pos+col?
  // I think pos+col then script, as this way scripts can inquire as to each
  // others (correct) position every frame

  comPosMan.Update();  // Use script from previous frame to update position+
  comCollisionMan.Update();  // Use newly generated position in collision (and
                             // change position if needed)
  comScriptMan.Update();  // Use input to run script and set next frame coords

  // Could conceivably use concurrency here; generate all collision events using
  // multiple threads, then process with a single thread

  // Render
  // Each of these could be put on their own thread and run concurrently
  // comCameraMan.    Update(); //Update cameras
  // Use new position to set camera values
  comCameraMan.Update();
  comLightMan.Update();   // Update lights and generate light map
  comSpriteMan.Update();  // Update sprites

  // execution now jumps to rendermanager
}

GameStateManager::GameStateManager(InputManager *input) : inputManager(input) {}

GameStateManager_Impl::GameStateManager_Impl(InputManager *input)
    : GameStateManager(input) {
  remapKey = "";
  swapState = false;
  mCurrentState = NULL;
  nextFrameStateScript = NULL;
  nextFrameState.reset();
}

int GameStateManager_Impl::StackSize() { return mGameStates.size(); }

bool GameStateManager_Impl::IsEmpty() { return mGameStates.empty(); }

GameState *GameStateManager_Impl::GetCurrentState() { return mCurrentState; }

void GameStateManager_Impl::Close() {
  while (mGameStates.empty() == false) {
    mGameStates.back()->Close();
    mGameStates.pop_back();
  }
  mCurrentState = NULL;
}

void GameStateManager_Impl::PushState(const std::shared_ptr<GameState> &state,
                                      const RSC_Script *script) {
  nextFrameState = state;
  nextFrameStateScript = script;
  swapState = false;
}

void GameStateManager_Impl::SwapState(const std::shared_ptr<GameState> &state,
                                      const RSC_Script *script) {
  PushState(state, script);
  swapState = true;
}

void GameStateManager_Impl::PushNextState() {
  if (nextFrameState.get() != NULL) {
    // Pop current state before pushing the new one
    if (swapState == true) {
      PopState();
    }
    mGameStates.push_back(nextFrameState);
    nextFrameState.reset();

    mCurrentState = mGameStates.back().get();
    mCurrentState->Init(nextFrameStateScript);
    nextFrameStateScript = NULL;
  }
}

void GameStateManager_Impl::PopState() { popState = true; }

void GameStateManager_Impl::PopTopState() {
  if (mGameStates.empty()) {
    return;
  }
  // Inform the state that it is being popped
  mGameStates.back()->Close();

  // Pop State, deleting it if this is the last shared_ptr to it
  mGameStates.pop_back();

  if (mGameStates.empty()) {
    mCurrentState = NULL;
    return;
  }

  mCurrentState = mGameStates.back().get();
  inputManager->SetEventDispatcher(&mCurrentState->eventDispatcher,
                                   &mCurrentState->input);

  mCurrentState->Resume();

  // Reset next state to push, in case both pop state and push state were called
  // in the same frame
  nextFrameState.reset();
  mCurrentState = mGameStates.back().get();
  nextFrameStateScript = NULL;
}

void GameStateManager_Impl::DrawPreviousState(GameState *gs) {
  GameState *prev = GetPreviousState(gs);
  if (prev != NULL) {
    prev->Draw();
  }
}

GameState *GameStateManager_Impl::GetPreviousState(GameState *gs) {
  // reverse iterator, as this function will most likely be called with a
  // pointer to the back of the vector
  for (auto i = mGameStates.rbegin(); i != mGameStates.rend(); i++) {
    if (gs == (i->get())) {
      return (i->get());
    }
  }
  return NULL;
}

void GameStateManager_Impl::HandleEvent(const Event *event) {
  if (mCurrentState != NULL) {
    mCurrentState->HandleEvent(event);
  }
}

bool GameStateManager_Impl::Update() {
  mCurrentState->entityMan.Cleanup();
  if (popState == false) {
    // push next state if one exists
    PushNextState();
  } else {
    PopTopState();
    popState = false;
  }

  if (mCurrentState != NULL) {
    if (!mCurrentState->Update()) {
      PopState();
      if (mGameStates.empty()) {
        return false;
      }
    }
    return true;
  }

  return false;
}

void GameStateManager_Impl::Draw() {
  if (mCurrentState != NULL) {
    mCurrentState->Draw();
  }
}

void GameState::SetMapHandleRenderableLayers(
    const std::map<MAP_DEPTH, TiledLayerGeneric *> &layers) {
  for (auto i = layers.begin(); i != layers.end(); i++) {
    auto layerType = i->second->layerType;
    TiledLayerGeneric *tiledLayer = i->second;
    if ((layerType == LAYER_TILE) or (layerType == LAYER_IMAGE)) {
      RenderableObject *layerRawPointer = NULL;
      if (i->second->layerType == LAYER_TILE) {
        auto layer = std::make_unique<RenderTileLayer>(
            &renderMan, (TiledTileLayer *)i->second);
        layerRawPointer = layer.get();

        mCurrentMapRenderableLayers[tiledLayer] = (std::move(layer));
      }

      else if (i->second->layerType == LAYER_IMAGE) {
        auto layer = std::make_unique<RenderImageLayer>(
            &renderMan, (TiledImageLayer *)i->second);
        layerRawPointer = layer.get();

        mCurrentMapRenderableLayers[tiledLayer] = (std::move(layer));
      }
      if (layerRawPointer != NULL) {
        if ((i->second->mShaderFrag != "") or (i->second->mShaderVert != "") or
            (i->second->mShaderGeo != "")) {
          auto shaderProgram = renderMan.LoadShaderProgram(
              i->second->mShaderVert, i->second->mShaderFrag);
          renderMan.LinkShaderProgram(shaderProgram.get());
          std::stringstream programName;
          programName << i->second->mShaderFrag << i->second->mShaderVert;

          std::unique_ptr<const RSC_GLProgram> constProgram =
              std::move(shaderProgram);
          K_ShaderProgramMan.LoadItem(programName.str(), constProgram);

          auto program = K_ShaderProgramMan.GetItem(programName.str());
          layerRawPointer->SetShaderProgram(program);
        }
      }
    }
  }
}

std::vector<EID> GameState::SetMapGetEntitiesUsingEntrances(
    const std::vector<std::unique_ptr<TiledObjectLayer>> &layers) {
  std::vector<EID> objectsUsingEntrance;

  for (auto ii = layers.begin(); ii != layers.end(); ii++) {
    for (auto objectIt = (*ii)->objects.begin();
         objectIt != (*ii)->objects.end(); objectIt++) {
      // Include object if it utilizes an entrance
      if ((objectIt)->second.useEntrance) {
        EID id = entityMan.NewEntity(objectIt->second.name);
        objectsUsingEntrance.push_back(id);
      }
    }
  }

  return objectsUsingEntrance;
}

// returns a data structure mapping tiled EIDS to engine EIDS
std::map<EID, EID> GameState::SetMapCreateEntitiesFromLayers(
    const std::vector<std::unique_ptr<TiledObjectLayer>> &layers) {
  std::map<EID, EID> tiledIDtoEntityID;  // This is for the purpose of linking
                                         // event listeners later

  for (auto ii = layers.begin(); ii != layers.end(); ii++) {
    for (auto objectIt = (*ii)->objects.begin();
         objectIt != (*ii)->objects.end(); objectIt++) {
      // New Entity
      // If the entity has a name, map that name to it's engine generated ID
      EID ent = entityMan.NewEntity(objectIt->second.name);

      // Map Tile map ID to Engine generated ID
      tiledIDtoEntityID[objectIt->first] = ent;

      // Set position
      comPosMan.AddComponent(ent);
      comPosMan.GetComponent(ent)->SetPositionLocal(
          Coord2df(objectIt->second.x, objectIt->second.y));

      // Add script Component and set to be initialized later during linking
      // stage after all entities have EIDs
      auto scripts = objectIt->second.scripts;
      auto prefabName = objectIt->second.prefabName;
      if ((!scripts.empty()) or (prefabName != "")) {
        comScriptMan.AddComponent(ent);
      }

      // Set up Light source (if applicable)
      /*
      if (objectIt->second.light == true) {
        comLightMan.AddComponent(ent);
      }
      */
    }
  }

  return tiledIDtoEntityID;
}

std::vector<std::string> GetScriptsFromString(std::string typeString) {
  if (typeString == "") {
    return std::vector<std::string>();
  }
  auto types = StringSplit(typeString.c_str(), ',');
  return types;
}

void GameState::SetMapLinkEntities(
    const std::vector<std::unique_ptr<TiledObjectLayer>> &layers,
    std::map<EID, EID> &tiledIDtoEntityID,
    const std::vector<EID> &objectsUsingEntrance) {
  /*
   * If a parent tiled ID is '0' then it doesn't have a parent
   * This mapping allows the algorithim to work without checking for '0' as a
   * special case
   * Naturally, this also means that there cannot be any entities with a tileID
   * of '0'
   */
  tiledIDtoEntityID[0] = 0;

  MapEntrance *correctEntrance = NULL;
  int correctEntranceX = 0;
  int correctEntranceY = 0;

  for (auto ii = layers.begin(); ii != layers.end(); ii++) {
    // second pass (Linking)
    EID entityID, listenerID;
    EID parent = 0;
    EID parentEID = 0;
    EID child = 0;
    for (auto objectIt = (*ii)->objects.begin();
         objectIt != (*ii)->objects.end(); objectIt++) {
      parent = objectIt->second.parent;
      child = tiledIDtoEntityID.find(objectIt->first)->second;
      parentEID = tiledIDtoEntityID.find(parent)->second;

      ASSERT(tiledIDtoEntityID.find(objectIt->first) !=
             tiledIDtoEntityID.end());
      ASSERT(tiledIDtoEntityID.find(parent) != tiledIDtoEntityID.end());

      // Initialize Script
      std::string &scriptsString = objectIt->second.scripts;

      // Map Names to EIDs
      if (objectIt->second.name != "") {
        AddNameEIDLookup(objectIt->second.name, child);
      }

      auto prefabName = objectIt->second.prefabName;
      auto scripts = GetScriptsFromString(scriptsString);
      if ((!scripts.empty()) or (prefabName != "")) {
        std::vector<std::string> scriptNames;
        if (prefabName != "") {
          prefabName = tiledProperties::object::PREFAB_PREFIX + prefabName;
          auto prefab = K_PrefabMan.GetLoadItem(prefabName, prefabName);
          // Prefab scripts will be run first
          if (prefab != NULL) {
            for (auto i = prefab->mScripts.begin(); i != prefab->mScripts.end();
                 i++) {
              scriptNames.push_back(*i);
            }
          }
        }
        for (auto i = scripts.begin(); i != scripts.end(); i++) {
          scriptNames.push_back(*i);
        }
        std::vector<const RSC_Script *> finalScripts;
        for (auto i = scriptNames.begin(); i != scriptNames.end(); i++) {
          auto scriptName = *i;
          auto script = K_ScriptMan.GetLoadItem(scriptName, scriptName);
          if (script != NULL) {
            finalScripts.push_back(script);
          }
        }

        // entities have been created so that parents can be assigned properly
        luaInterface.RunScript(child, finalScripts, (*ii)->GetDepth(),
                               parentEID, objectIt->second.name,
                               &objectIt->second, NULL);
      }
      // Event Linking
      // Can move into scriptManager
      for (auto eventSource = objectIt->second.eventSources.begin();
           eventSource != objectIt->second.eventSources.end(); eventSource++) {
        entityID = tiledIDtoEntityID.find((*eventSource))->second;
        listenerID = tiledIDtoEntityID.find(objectIt->first)->second;

        ComponentScript *listenerScript = comScriptMan.GetComponent(listenerID);
        ComponentScript *senderScript = comScriptMan.GetComponent(entityID);
        if ((listenerScript == NULL) or (senderScript == NULL)) {
          std::stringstream ss;
          ss << "[Map Loader: " << mCurrentMap->GetMapName() << "]"
             << " Couldn't listen to event from TiledID " << *eventSource
             << "     Listener TiledID is: " << objectIt->first;
          LOG_INFO(ss.str());
          continue;
        }
        senderScript->EventLuaAddObserver(listenerScript);
      }

      // Parent Linking
      if (parent != 0) {
        ComponentScript *scriptComp = comScriptMan.GetComponent(child);

        // if entity has a script, it's up to the script to decide how to handle
        // the parent, else it's assigned to all components
        if (scriptComp == NULL) {
          entityMan.SetParent(child, parentEID);
        }
      }
    }
  }

  // Set all objects that use entrances to their proper entrance
  if (correctEntrance != NULL) {
    for (auto i = objectsUsingEntrance.begin(); i != objectsUsingEntrance.end();
         i++) {
      comPosMan.GetComponent(*i)->SetPositionLocal(
          Coord2df(correctEntranceX, correctEntranceY));
    }
  }
}

void GameState::SetMapNextFrame(const RSC_Map *m, unsigned int entranceID) {
  nextMap = m;
  nextMapEntrance = entranceID;
}

const std::vector<EID> *GameState::GetEIDFromName(
    const std::string &name) const {
  auto i = mNameLookup.find(name);
  if (i == mNameLookup.end()) {
    return NULL;
  }

  return &i->second;
}

bool GameState::SetCurrentMap(const RSC_Map *m, unsigned int entranceID) {
  if (m == NULL) {
    LOG_ERROR("GameState::SetCurrentTiledMap was passed a NULL Pointer");
    return false;
  }

  std::stringstream ss;
  ss << "Changing Map To: " << m->GetMapName();
  LOG_INFO(ss.str());

  mEntitiesToCreate.clear();

  // Unload all layers from last map
  mCurrentMapRenderableLayers.clear();
  // Clear all entites from current state
  entityMan.ClearAllEntities();
  // Actually delete all entities
  entityMan.Cleanup();
  mNameLookup.clear();

  // Copy map passed
  mCurrentMap.reset();
  mCurrentMap = std::make_unique<RSC_MapImpl>(*((RSC_MapImpl *)(m)));

  TiledData *td = (mCurrentMap->GetTiledData());
  SetMapHandleRenderableLayers(td->tiledRenderableLayers);

  const std::vector<std::unique_ptr<TiledObjectLayer>> &layers =
      td->tiledObjectLayers;
  auto tiledIDtoEntityID = SetMapCreateEntitiesFromLayers(layers);
  auto objectsUsingEntrance = SetMapGetEntitiesUsingEntrances(layers);

  SetMapLinkEntities(layers, tiledIDtoEntityID, objectsUsingEntrance);

  return true;
}

EID GameState::CreateLuaEntity(std::unique_ptr<EntityCreationPacket> p) {
  // New Entity
  EID newEID = entityMan.NewEntity();
  p->mNewEID = newEID;

  std::vector<const RSC_Script *> scripts;

  // Get script Data
  for (auto i = p->mScriptNames.begin(); i != p->mScriptNames.end(); i++) {
    std::string scriptName = *i;
    const RSC_Script *scriptData =
        K_ScriptMan.GetLoadItem(scriptName, scriptName);

    if (scriptData == NULL) {
      std::stringstream ss;
      ss << "LuaInterface::EntityNew; Couldn't Load Script Named: "
         << scriptName;
      LOG_ERROR(ss.str());
      ASSERT(ss.str() == "");
    }

    scripts.push_back(scriptData);
  }

  if (scripts.empty()) {
    return 0;
  }

  mEntitiesToCreate.push_back(
      std::pair<std::vector<const RSC_Script *>,
                std::unique_ptr<EntityCreationPacket>>(scripts, std::move(p)));

  return newEID;
}

void GameState::AddNameEIDLookup(const std::string &name, EID id) {
  auto vecIt = mNameLookup.find(name);
  // Create new vector for this name if one doesn't exist
  if (vecIt == mNameLookup.end()) {
    mNameLookup[name] = std::vector<EID>();
  }

  mNameLookup[name].push_back(id);
}

void GameState::CreateNewEntities() {
  if (mEntitiesToCreate.empty() == true) {
    return;
  }

  for (auto newEnt = mEntitiesToCreate.begin();
       newEnt != mEntitiesToCreate.end(); newEnt++) {
    auto scripts = std::get<0>(*newEnt);
    auto packet = std::get<1>(*newEnt).get();

    // New Position Component for Entity
    comPosMan.AddComponent(packet->mNewEID);
    comPosMan.GetComponent(packet->mNewEID)->SetPositionLocal(packet->mPos);

    // Add script component and run script
    comScriptMan.AddComponent(packet->mNewEID);

    if (luaInterface.RunScript(packet->mNewEID, scripts, packet->mDepth,
                               packet->mParent, packet->mEntityName, NULL,
                               &packet->mPropertyTable) == false) {
      LOG_ERROR("Couldn't Create a new Entity");
      continue;
    }
    AddNameEIDLookup(packet->mEntityName, packet->mNewEID);
  }

  mEntitiesToCreate.clear();
}

std::string GameState::GetScriptName(EID id) const {
  auto scriptComponent = comScriptMan.GetConstComponent(id);
  if (scriptComponent == NULL) {
    return "";
  }

  return scriptComponent->GetScriptName();
}

void GameState::DeleteMapLayer(TiledLayerGeneric *layer) {
  if (mCurrentMap.get() != NULL) {
    mCurrentMapRenderableLayers.erase(layer);
    mCurrentMap->GetTiledData()->DeleteLayer(layer);
  }
}
