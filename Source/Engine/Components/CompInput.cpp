#include "CompInput.h"
#include "../Kernel.h"

using namespace luabridge;

ComponentInput::ComponentInput(InputManager::KeyMapping* keys, EID id, const std::string& logName) : BaseComponent(id, logName), keyMapping(keys){
    mEntityID=id;
}
ComponentInput::~ComponentInput(){

}

void ComponentInput::Update(){}

void ComponentInput::HandleEvent(const Event* event){
}

void ComponentInput::ListenForInput(std::string keyName){
    (*keyMapping)[keyName].insert(mEntityID);
}

void ComponentInputManager::AddComponent(EID id, EID parent){
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    auto input = make_unique<ComponentInput>(keyMapping.get(), id, logFileName);
    componentList[id] = std::move(input);
}

void ComponentInputManager::SetDependency(std::shared_ptr<InputManager::KeyMapping> keys){
    keyMapping = keys;
}

ComponentInputManager::ComponentInputManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_INPUT", e){

}
