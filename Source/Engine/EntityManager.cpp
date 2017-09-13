#include "EntityManager.h"
#include "StateManager.h"

EntityManager::EntityManager(GameStateManager* gsm) : mStateManager(gsm){
	ASSERT(gsm != NULL);
	entityNumber=EID_MIN;
	mFlagDeleteAll=false;
	entityCount = 0;
}

void EntityManager::DeleteEntity(EID id){
    deadEntities.push_back(id);
	entityCount -= 1;
}

unsigned int EntityManager::GetEntityCount(){
	return entityCount;
}

void EntityManager::DispatchEvent(const Event* event){
    for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
        i->second->HandleEvent(event);
    }
}

void EntityManager::BroadcastEvent(const Event* event){
	//Send to state
	mStateManager->HandleEvent(event);
    for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
        i->second->BroadcastEvent(event);
    }
}

void EntityManager::Cleanup(){
    if(deadEntities.empty()){return;}

    for(auto i = deadEntities.begin(); i!=deadEntities.end(); i++){
        EID id=*i;

		//Let all entities know that this entity is about to be deleted
		Event event (id, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "[DELETED]");
		BroadcastEvent(&event);


        for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
            i->second->DeleteComponent(id);
        }

        if(EIDToName.find(id)!=EIDToName.end()){
            std::string entityName = EIDToName[id];
            EIDToName.erase(id);
            nameToEID.erase(entityName);
        }
    }

    if(mFlagDeleteAll){
        mFlagDeleteAll=false;
        ClearNameMappings();
    }
    deadEntities.clear();
}

void EntityManager::RegisterComponentManager(BaseComponentManager* manager, int order){
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
	entityCount++;
    entityNumber++;
    return newEntityID;
}

void EntityManager::ClearAllEntities(){
    for(EID i = EID_MIN; i < entityNumber; i++){
        DeleteEntity(i);
    }
    mFlagDeleteAll = true;
    entityNumber = EID_MIN;
}

void EntityManager::ClearAllReservedEntities(){
    for(EID i = 0; i < EID_MIN; i++){
        DeleteEntity(i);
    }
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

void EntityManager::SetParent(EID child, EID parent){
	for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
		i->second->SetParent(child, parent);
	}
}
