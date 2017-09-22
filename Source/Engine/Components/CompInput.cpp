#include "CompInput.h"
#include "../Kernel.h"

using namespace luabridge;

ComponentInput::ComponentInput(InputManager::KeyMapping* keys, EID id, ComponentInputManager* manager) : BaseComponent(id, manager), keyMapping(keys){
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

std::unique_ptr<ComponentInput> ComponentInputManager::ConstructComponent (EID id, ComponentInput* parent){
    auto input = make_unique<ComponentInput>(keyMapping.get(), id, this);
    return std::move(input);
}

void ComponentInputManager::SetDependency(std::shared_ptr<InputManager::KeyMapping> keys){
    keyMapping = keys;
}

ComponentInputManager::ComponentInputManager(EventDispatcher* e) : BaseComponentManager_Impl(e){

}
