template <class T>
BaseComponentManager_Impl<T>::BaseComponentManager_Impl(EventDispatcher* e)
	: eventDispatcher(e){

}

template <class T>
int BaseComponentManager_Impl<T>::GetComponentCount(){
    return componentList.size();
}

template <class T>
int BaseComponentManager_Impl<T>::GetActiveComponentCount(){
    return activeComponents.size();
}

template <class T>
void BaseComponentManager_Impl<T>::DeleteAllComponents(){
    componentList.clear();
	activeComponents.clear();
}

template <class T>
void BaseComponentManager_Impl<T>::DeleteComponent(EID id){
    //auto comp=componentList.find(id);
    //if(comp==componentList.end()){return;}

	componentList.erase(id);
	activeComponents.erase(id);
}

template <class T>
bool BaseComponentManager_Impl<T>::HasComponent(EID id){
    if(componentList.find(id)!=componentList.end()){
        return true;
    }
    return false;
}

template <class T>
bool BaseComponentManager_Impl<T>::IsActive(EID id){
	return (activeComponents.find(id) != activeComponents.end());
}

template <class T>
void BaseComponentManager_Impl<T>::UpdateComponent(T* child){
	//Exit if already updated this frame
	if(child->updatedThisFrame == true){return;}

	//Update parent before child
	auto parent = child->GetParent();
	if(parent != NULL){
		auto parentEID = parent->GetEID();
		auto parentIterator = activeComponents.find(parentEID);	
		if(parentIterator != activeComponents.end()){
			//Recurse update function with parent
			UpdateComponent(parentIterator->second);
		}
	}

	//Update child
	child->Update();
	child->updatedThisFrame = true;
}

template <class T>
void BaseComponentManager_Impl<T>::Update(){
	//Update all entities
    for(auto i=activeComponents.begin(); i!=activeComponents.end(); i++){
		UpdateComponent(i->second);
    }

	//Reset all 'updatedThisFrame' bits
	for(auto i = activeComponents.begin(); i != activeComponents.end(); i++){
		i->second->updatedThisFrame = false;
	}
}

template <class T>
void BaseComponentManager_Impl<T>::HandleEvent(const Event* event){
	EID recieverEID = event->reciever;
    BaseComponent* comp=GetComponent(recieverEID);
    if(comp==NULL){return;}

	ProcessEvent(event);

    comp->HandleEvent(event);
}
template <class T>
void BaseComponentManager_Impl<T>::ProcessEvent(const Event* event){
	EID recieverEID = event->reciever;
	if(recieverEID == EID_ALLOBJS){return;}
    BaseComponent* comp=GetComponent(recieverEID);
    if(comp==NULL){return;}

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

template <class T>
void BaseComponentManager_Impl<T>::BroadcastEvent(const Event* event){
	ProcessEvent(event);
    for(auto i = activeComponents.begin(); i != activeComponents.end(); i++){
		if(i->second->GetEID() != event->sender){
			i->second->HandleEvent(event);
		}
	}
}

template <class T>
T* BaseComponentManager_Impl<T>::GetComponent(EID id){
    auto i=componentList.find(id);
    if(i!=componentList.end()){
        return i->second.get();
    }
    return NULL;
}

template <class T>
void BaseComponentManager_Impl<T>::AddComponent(std::unique_ptr<T> comp){
	if(comp.get() == NULL){
		return;	
	}
    auto id = comp->GetEID();

	//This shouldn't happen
    ASSERT( componentList.find(id) == componentList.end() );
    if(componentList.find(id) != componentList.end() ){
		return;
	}

    componentList[id]=std::move(comp);
	ActivateComponent(id);
}

template <class T>
void BaseComponentManager_Impl<T>::AddComponent(EID id, EID parent){
	T* parentPointer = NULL;

	auto i = componentList.find(id);
    if(i!=componentList.end()){
		parentPointer = i->second.get();
	}

	AddComponent(ConstructComponent(id, parentPointer));
}

template <class T>
void BaseComponentManager_Impl<T>::SetParent(EID child, EID parent){
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

template <class T>
void BaseComponentManager_Impl<T>::ActivateComponent(EID id){
	auto comp = componentList.find(id);
	if(comp != componentList.end()){
		activeComponents[id] = comp->second.get();
	}
}

template <class T>
void BaseComponentManager_Impl<T>::DeactivateComponent(EID id){
	auto comp = activeComponents.find(id);
	if(comp != activeComponents.end()){
		activeComponents.erase(comp);
	}
}
