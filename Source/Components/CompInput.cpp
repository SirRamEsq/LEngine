#include "CompInput.h"
#include "../Kernel.h"

//only use if not mocked
#ifndef TEST_ComponentCamera_MOCK

using namespace luabridge;

ComponentInput::ComponentInput(EID id, ComponentScript* script, const std::string& logName) : BaseComponent(id, logName){
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
    K_InputMan.ListenForInput(keyName, mEntityID);
}

void ComponentInputManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentInput* input=new ComponentInput(id, (ComponentScript*)Kernel::stateMan.GetCurrentState()->comScriptMan.GetComponent(id), logFileName);
    componentList[id]=input;
}

ComponentInputManager::ComponentInputManager() : BaseComponentManager("LOG_COMP_INPUT"){

}

#endif // TEST_ComponentCamera_MOCK
