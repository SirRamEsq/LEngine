#include "BaseComponent.h"

BaseComponent::BaseComponent(EID id, const std::string& logName, BaseComponent* p)
	: mEntityID(id), logFileName(logName), parent(p){
		eventCallback = NULL;
}

BaseComponent::~BaseComponent(){
    //Event* event= new Event(mEntityID, mEntityID, MSG_COMP_DELETED, this);
    //K_EventMan.DispatchEvent(event);
}

void BaseComponent::SetEventCallbackFunction(EventFunction f){
    eventCallback=f;
}

void BaseComponent::HandleEvent(const Event* event){
    if(eventCallback!=NULL){
        eventCallback(event);
    }
}


int BaseComponentManager::GetComponentCount(){
    return componentList.size();
}

void BaseComponentManager::DeleteAllComponents(){
    auto comp=componentList.begin();
    for(auto comp = componentList.begin(); comp != componentList.end(); comp++){
        delete comp->second;
    }
    componentList.clear();
}

void BaseComponentManager::DeleteComponent(EID id){
    auto comp=componentList.find(id);
    if(comp!=componentList.end()){
		Event event (id, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "[DELETED]");
		dependencyEventDispatcher->DispatchEvent(event);

        componentList.erase(id)
        delete comp->second;
    }
}

bool BaseComponentManager::HasComponent(EID id){
    if(componentList.find(id)!=componentList.end()){
        return true;
    }
    return false;
}

void BaseComponentManager::UpdateComponent(BaseComponent* child){
	//Exit if already updated this frame
	if(child->updatedThisFrame == true){return;}

	//Update parent before child
	if(child->parent != NULL){
		auto parent = child->parent;
		if(parent->updatedThisFrame == false){
			UpdateComponent(parent);
		}
	}

	//Update child
	child->Update();
	child->updatedThisFrame = true;
}

void BaseComponentManager::Update(){
	//Update all entities
    for(i=componentList.begin(); i!=componentList.end(); i++){
		UpdateComponent(i->second);
    }

	//Reset all 'updatedThisFrame' bits
	for(auto i = componentList.begin(); i !+ componentList.end(); i++){
		i->second->updatedThisFrame = false;
	}
}

void BaseComponentManager::HandleEvent(const Event* event){
	EID recieverEID = event->reciever;
    BaseComponent* comp=GetComponent(recieverEID);
    if(comp==NULL){return;}

    comp->HandleEvent(event);

	if(event->message == Event::MSG::ENTITY_DELETED){
		//if component has a parent, it needs to be changed if the parent was deleted
		if(comp->parent != NULL){
			if(comp->parent->GetEID() == event->sender){
				//Set parent to NULL
				SetParent(recieverEID, 0);
			}
		}
	}
}

void BaseComponentManager::BroadcastEvent(const Event* event){
    for(auto i = componentList.begin(); i != componentList.end(); i++){
		if(i->second->GetEID() != event->sender){
			i->second->HandleEvent(event);
		}
	}
}

BaseComponent* BaseComponentManager::GetComponent(EID id){
    compMap::iterator i=componentList.find(id);
    if(i!=componentList.end()){
        return i->second;
    }
    return NULL;
}

void BaseComponentManager::AddComponent(std::unique_ptr<BaseComponent> comp){
    auto id = comp->GetEID();
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    componentList[id]=comp.release();
}

void BaseComponentManager::SetParent(EID child, EID parent){
	if(child==0){return;}

	auto childComponent = componentList.find(child);
	if(childComponent == componentList.end()){return;}
	if(parent==0){
		childComponent->parent = NULL;
		return;
	}

	auto parentComponent = componentList.find(parent);
	if(parentComponent == componentList.end()){return;}

	childComponent->parent = parentComponent;
}
