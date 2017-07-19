#include "BaseComponent.h"

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
    compMapIt comp=componentList.begin();
    for(;comp!=componentList.end();comp++){
        delete comp->second;
    }
    componentList.clear();
}

void BaseComponentManager::DeleteComponent(EID id){
    compMapIt comp=componentList.find(id);
    if(comp!=componentList.end()){
        componentList.erase(id);
        delete comp->second;
    }
}

bool BaseComponentManager::HasComponent(EID id){
    if(componentList.find(id)!=componentList.end()){
        return true;
    }
    return false;
}

void BaseComponentManager::Update(){
    compMapIt i=componentList.begin();
    for(; i!=componentList.end(); i++){
        i->second->Update();
    }
}

void BaseComponentManager::HandleEvent(const Event* event){
    BaseComponent* comp=GetComponent(event->reciever);
    if(comp==NULL){return;}
    comp->HandleEvent(event);
}

void BaseComponentManager::BroadcastEvent(const Event* event){
    for(auto i = componentList.begin(); i != componentList.end(); i++){
		i->second->HandleEvent(event);
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
