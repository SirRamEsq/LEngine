#include "EntityManager.h"
#include "StateManager.h"
#include "Kernel.h"

EntityManager::EntityManager(GameStateManager* gsm) : mStateManager(gsm){
	ASSERT(gsm != NULL);
	entityNumber=EID_MIN;
	entityCount = 0;
}

void EntityManager::DeleteEntity(EID id){
	deadEntities.insert(id);
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

		reclaimedEIDs.push_back(id);
    }

    deadEntities.clear();
}

void EntityManager::RegisterComponentManager(BaseComponentManager* manager, int order){
    if(componentsRegistered.find(order)!=componentsRegistered.end()){
        std::stringstream ss;
        ss << "Couldn't register component with order: '" << order << "' order id already taken.";
        throw Exception(ss.str());
    }
	
	//Assert that this order not already taken
	ASSERT( componentsRegistered.find(order) == componentsRegistered.end() );

    componentsRegistered[order] = manager;
}

EID EntityManager::NewEntity(const std::string& entityName){
	EID newEntityID = 0;

	//First check if there are any reclaimed eids to use
	if(!reclaimedEIDs.empty()){
		newEntityID = reclaimedEIDs.back();
		reclaimedEIDs.pop_back();
	}
	//If there are no old eids to use, make a new one
	else{
		entityNumber++;
		newEntityID = entityNumber;
	}

	MapNameToEID(newEntityID, entityName);
	//Increment number of living entities
	entityCount++;

    return newEntityID;
}

void EntityManager::MapNameToEID(EID eid, const std::string& entityName){
    if(entityName!=""){
		//Stop if in debug mode, this shouldn't happen
		ASSERT (nameToEID.find(entityName) == nameToEID.end());
		//Defensive programming if not in debug mode
        if(nameToEID.find(entityName) == nameToEID.end()){
            nameToEID[entityName]	= eid;
            EIDToName[eid] 			= entityName;
        }
	}
}

void EntityManager::ClearAllEntities(){
    for(EID i = EID_MIN; i < entityNumber; i++){
        DeleteEntity(i);
    }
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

void EntityManager::ActivateAllEntities(){

}

void EntityManager::ActivateAllEntitiesExcept(std::vector<EID> entities){
	
}

void EntityManager::DeactivateAllEntitiesExcept(std::vector<EID> entities){
	
}

void EntityManager::DeactivateAllEntities(){

}

void EntityManager::Activate(std::vector<EID> entities){
	for(auto i = entities.begin(); i!= entities.end(); i++){
		for(auto comp = componentsRegistered.begin(); comp != componentsRegistered.end(); comp++){
			comp->second->ActivateComponent(*i);
		}
	}
}

void EntityManager::Deactivate(std::vector<EID> entities){
	for(auto i = entities.begin(); i!= entities.end(); i++){
		for(auto comp = componentsRegistered.begin(); comp != componentsRegistered.end(); comp++){
			comp->second->DeactivateComponent(*i);
		}
	}
}
