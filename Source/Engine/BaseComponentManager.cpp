BaseComponentManager(const std::string& logName, EventDispatcher* e)
	: logFileName(logName), eventDispatcher(e){

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

	componentList.erase(id);
}

bool BaseComponentManager::HasComponent(EID id){
    if(componentList.find(id)!=componentList.end()){
        return true;
    }
    return false;
}

void BaseComponentManager::UpdateComponent(T* child){
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

void BaseComponentManager::AddComponent(std::unique_ptr<T> comp){
    auto id = comp->GetEID();
    auto i = componentList.find(id);
    if(i!=componentList.end()){return;}
    componentList[id]=std::move(comp);
}

void ComponentScriptManager::AddComponent(EID id, EID parent){
	T* parentPointer = NULL;

    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}

	auto parentComp = componentList.find(id);
    if(i!=componentList.end()){
		parentPointer = i->second.get();
	}

    auto script = make_unique<T>(id, logFileName, parentPointer);
	script->mManager = this;
    componentList[id] = std::move(script);
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
