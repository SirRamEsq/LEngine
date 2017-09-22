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

BaseComponent* BaseComponent::GetParent(){
	return parent;
}

