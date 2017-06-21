#include "../../../Source/Components/CompInput.h"
#include "../../../Source/Kernel.h"

//Only use this file if mocking
#ifdef TEST_ComponentCamera_MOCK

ComponentInput::ComponentInput(EID id, const std::string& logName) : BaseComponent(id, logName){

}

void ComponentInput::Update(){}

void ComponentInput::HandleEvent(const Event* event){
    /*bool keyup;
    std::string function;
    if( (keyup = (event->message==MSG_KEYUP) ) or (event->message == MSG_KEYDOWN) ){
        if(myScript==NULL){return;}
        if(keyup){function="KeyUp";}
        else{function="KeyDown";}

        //luabind::call_function<int>(myScript->lState, function.c_str(), ((std::string*)event->extradata)->c_str());
        LuaRef fKeyPress = getGlobal(myScript->lState, function.c_str());
        fKeyPress( ( (std::string*)event->extradata )->c_str() );
    }*/
}

void ComponentInput::ListenForInput(std::string keyName){
    //K_InputMan.ListenForInput(keyName, mEntityID);
}

void ComponentInputManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentInput* input=new ComponentInput(id, logFileName);
    componentList[id]=input;
}

ComponentInputManager::ComponentInputManager() : BaseComponentManager("LOG_COMP_INPUT"){

}

#endif // TEST_ComponentCamera_MOCK
