#include "BaseComponent.h"
#include "Kernel.h"
#include "Errorlog.h"

BaseComponent::BaseComponent(EID id, const std::string& logName, BaseComponent* p)
	: mEntityID(id), logFileName(logName), parent(p){
		eventCallback = NULL;
		updatedThisFrame = false;
}

BaseComponent::~BaseComponent(){
}

void BaseComponent::SetEventCallbackFunction(EventFunction f){
    eventCallback=f;
}

void BaseComponent::HandleEvent(const Event* event){
	//If parent was deleted, set parent to NULL
	if(event->message == Event::MSG::ENTITY_DELETED){
		if(parent != NULL){
			if(event->sender == parent->GetEID()){
				SetParent(NULL);
			}
		}
	}
    if(eventCallback!=NULL){
        eventCallback(event);
    }
}

void BaseComponent::SetParent(BaseComponent* p){
	parent = p;
}

void BaseComponent::SetParentEID(EID p){
	if(p == 0){
		SetParent(NULL);
		return;
	}
		
	if(mManager->HasComponent(p)==false){
		std::stringstream ss;
		ss << "Parent with eid " << p << " Doesn't Exist";
		K_Log.Write(ss.str());
		SetParent(NULL);
		return;
	}
	SetParent(mManager->GetComponent(p));
}

BaseComponent* BaseComponent::GetParent(){
	return parent;
}

int BaseComponentManager::GetComponentCount(){
    return componentList.size();
}

void BaseComponentManager::DeleteAllComponents(){
    auto comp=componentList.begin();
    componentList.clear();
}

void BaseComponentManager::DeleteComponent(EID id){
    auto comp=componentList.find(id);
    if(comp==componentList.end()){return;}

	Event event (id, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "[DELETED]");
	BroadcastEvent(&event);

	componentList.erase(id);
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
	auto parent = child->GetParent();
	if(parent != NULL){
		//Recurse update function with parent
		UpdateComponent(parent);
	}

	//Update child
	child->Update();
	child->updatedThisFrame = true;
}

void BaseComponentManager::Update(){
	//Update all entities
    for(auto i=componentList.begin(); i!=componentList.end(); i++){
		UpdateComponent(i->second.get());
    }

	//Reset all 'updatedThisFrame' bits
	for(auto i = componentList.begin(); i != componentList.end(); i++){
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
		auto parent = comp->GetParent();
		if(parent != NULL){
			if(parent->GetEID() == event->sender){
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
    auto i=componentList.find(id);
    if(i!=componentList.end()){
        return i->second.get();
    }
    return NULL;
}

void BaseComponentManager::AddComponent(std::unique_ptr<BaseComponent> comp){
    auto id = comp->GetEID();
    auto i = componentList.find(id);
    if(i!=componentList.end()){return;}
    componentList[id]=std::move(comp);
}

void BaseComponentManager::SetParent(EID child, EID parent){
	if(child==0){return;}

	auto childComponent = componentList.find(child);
	if(childComponent == componentList.end()){return;}
	if(parent==0){
		childComponent->second->SetParent(NULL);
		return;
	}

	auto parentComponent = componentList.find(parent);
	if(parentComponent == componentList.end()){return;}

	childComponent->second->SetParent(parentComponent->second.get());
}
