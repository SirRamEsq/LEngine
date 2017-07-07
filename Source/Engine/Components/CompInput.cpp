#include "CompInput.h"
#include "../Kernel.h"

using namespace luabridge;

ComponentInput::ComponentInput(InputManager::KeyMapping* keys, EID id, ComponentScript* script, const std::string& logName) : BaseComponent(id, logName){
    mEntityID=id;
    myScript=script;
}

void ComponentInput::Update(){}


void ComponentInput::HandleEvent(const Event* event){
    bool keyup;
    std::string function;
    if( (keyup = (event->message==MSG_KEYUP) ) or (event->message == MSG_KEYDOWN) ){
        if(myScript==NULL){return;}
        if(keyup){function="KeyUp";}
        else{function="KeyDown";}

        //luabind::call_function<int>(myScript->lState, function.c_str(), ((std::string*)event->extradata)->c_str());
        LuaRef fKeyPress = getGlobal(myScript->lState, function.c_str());
        fKeyPress( ( (std::string*)event->extradata )->c_str() );
    }
}

void ComponentInput::ListenForInput(std::string keyName){
    (*keyMapping)[keyName].insert(mEntityID);
}

void ComponentInputManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentInput* input=new ComponentInput(keyMapping.get(), id, (ComponentScript*)Kernel::stateMan.GetCurrentState()->comScriptMan.GetComponent(id), logFileName);
    componentList[id]=input;
}

void ComponentInputManager::SetDependency(std::shared_ptr<InputManager::KeyMapping> keys){
    keyMapping = keys;
}

ComponentInputManager::ComponentInputManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_INPUT", e){

}
