#include "EntityManager.h"

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
    for(auto i = componentsRegistered.begin(); i!= componentsRegistered.end(); i++){
        i->second->BroadcastEvent(event);
    }
}

void EntityManager::Cleanup(){
    //GameState* state = Kernel::stateMan.GetCurrentState();
    if(mFlagDeleteAll){
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

void EntityManager::RegisterComponentManager(BaseComponentManager* manager, int order){
    if(componentsRegistered.find(order)!=componentsRegistered.end()){
        std::stringstream ss;
        ss << "Couldn't register component with order: '" << order << "' order id already taken.";
        throw Exception(ss.str());
    }

    componentsRegistered[order] = manager;
}

EID EntityManager::NewEntity(EID parent, const std::string& entityName){
    EID newEntityID = entityNumber + 1;
    if(entityName!=""){
        if(nameToEID.find(entityName)==nameToEID.end()){
            nameToEID[entityName]  = newEntityID;
            EIDToName[newEntityID] = entityName;
        }
    }
	if(parent!=0){
		/// \TODO keep track of parents and update them before their children
	}
	entityCount++;
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
