#include "BaseComponent.h"
#include "Kernel.h"

BaseComponent::~BaseComponent(){
    //Event* event= new Event(mEntityID, mEntityID, MSG_COMP_DELETED, this);
    //K_EventMan.DispatchEvent(event);
}
