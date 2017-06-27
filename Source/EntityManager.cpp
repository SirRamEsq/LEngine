#include "EntityManager.h"

void EntityManager::DeleteEntity(EID id){
    deadEntities.push_back(id);
}

void EntityManager::DispatchEvent(const Event* event){
    /*GameState* state = Kernel::stateMan.GetCurrentState();
    state->comParticleMan    .HandleEvent(event);
    state->comCollisionMan   .HandleEvent(event);
    state->comInputMan       .HandleEvent(event);
    state->comPosMan         .HandleEvent(event);
    state->comScriptMan      .HandleEvent(event);
    state->comSpriteMan      .HandleEvent(event);
    state->comCameraMan      .HandleEvent(event);
    state->comLightMan       .HandleEvent(event);*/

    for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
        i->second->HandleEvent(event);
    }
}

void EntityManager::Cleanup(){
    //GameState* state = Kernel::stateMan.GetCurrentState();
    if(mFlagDeleteAll){
        /*state->comParticleMan    .DeleteAllComponents();
        state->comCollisionMan   .DeleteAllComponents();
        state->comInputMan       .DeleteAllComponents();
        state->comPosMan         .DeleteAllComponents();
        state->comScriptMan      .DeleteAllComponents();
        state->comSpriteMan      .DeleteAllComponents();
        state->comCameraMan      .DeleteAllComponents();
        state->comLightMan       .DeleteAllComponents();*/

        for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
            i->second->DeleteAllComponents();
        }

        mFlagDeleteAll=false;
        deadEntities.clear();
        ClearNameMappings();

        return;
    }
    if(deadEntities.empty()){return;}

    std::vector<EID>::iterator i = deadEntities.begin();
    EID id;
    for(; i!=deadEntities.end(); i++){
        id=*i;
        /*state->comParticleMan    .DeleteComponent(id);
        state->comCollisionMan   .DeleteComponent(id);
        state->comInputMan       .DeleteComponent(id);
        state->comPosMan         .DeleteComponent(id);
        state->comScriptMan      .DeleteComponent(id);
        state->comSpriteMan      .DeleteComponent(id);
        state->comCameraMan      .DeleteComponent(id);
        state->comLightMan       .DeleteComponent(id);*/
        for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
            i->second->DeleteComponent(id);
        }

        if(EIDToName.find(id)!=EIDToName.end()){
            std::string entityName = EIDToName[id];
            EIDToName.erase(id);
            nameToEID.erase(entityName);
        }
    }
    deadEntities.clear();
}

void EntityManager::RegisterComponentManager(int order, BaseComponentManager* manager){
    if(componentsRegistered.find(order)!=componentsRegistered.end()){
        std::stringstream ss;
        ss << "Couldn't register component with order: '" << order << "' order id already taken.";
        throw Exception(ss.str());
    }

    componentsRegistered[order] = manager;
}

EID EntityManager::NewEntity(const std::string& entityName){
    EID newEntityID = entityNumber + 1;
    if(entityName!=""){
        if(nameToEID.find(entityName)==nameToEID.end()){
            nameToEID[entityName]  = newEntityID;
            EIDToName[newEntityID] = entityName;
        }
    }
    entityNumber++;
    return newEntityID;
}

void EntityManager::ClearAllEntities(){
    for(EID i=EID_MIN; i<entityNumber; i++){
        DeleteEntity(i);
    }
    mFlagDeleteAll=true;
    entityNumber=EID_MIN;
}

void EntityManager::ClearNameMappings(){
    nameToEID.clear();
    EIDToName.clear();
}

EID EntityManager::GetEIDFromName(const std::string& name) const {
    std::map<std::string, EID>::const_iterator i=nameToEID.find(name);
    if(i==nameToEID.end()){return 0;}

    return i->second;
}
