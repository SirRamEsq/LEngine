#include "../../../Source/Components/CompScript.h"

#include "../../../Source/Kernel.h"
#include "../../../Source/Components/CompCollision.h"
#include <sstream>

#ifdef TEST_ComponentScript_MOCK

using namespace luabridge;

void RunInit(){
    //Push function on the stack
}

ComponentScript::ComponentScript(EID id, const std::string& logName)
: BaseComponent(id, logName), lInterface(NULL), scriptPointer(lState)
{

}

ComponentScript::~ComponentScript(){
}

void ComponentScript::SetScriptPointerOnce(luabridge::LuaRef lp){
    if(!scriptPointer.isNil()){
        std::stringstream ss;
        ss << "LuaPointer already set in script " << scriptName << " with EID " << mEntityID;
        throw LEngineException(ss.str());
    }
    scriptPointer = lp;
}

void ComponentScript::EventLuaBroadcastEvent (const std::string& event){
    /*for(auto it=mEventLuaObservers.begin(); it!=mEventLuaObservers.end(); it++){
        Event e(GetEID(), it->first, MSG_LUA_EVENT);
        e.eventDescription=event;
        it->second->HandleEvent(&e);
    }*/
}
void ComponentScript::EventLuaAddObserver    (ComponentScript* script){
/*    if(mEventLuaObservers.find(script->GetEID())==mEventLuaObservers.end()){
        mEventLuaObservers[script->GetEID()]=script;
    }*/
}
void ComponentScript::EventLuaRemoveObserver (EID id){
    /*if(mEventLuaObservers.find(id)!=mEventLuaObservers.end()){
        mEventLuaObservers.erase(id);
    }*/
}

/*RenderText* ComponentScript::RenderObjectText(int x, int y, const std::string& text, bool abss){
    RenderText* rt= new RenderText(x,y,text, abss);
    mRenderableObjects.insert(rt);
    return rt;
}

RenderLine* ComponentScript::RenderObjectLine(int x, int y, int xx, int yy){
    RenderLine* rl=new RenderLine(x,y,xx, yy);
    mRenderableObjects.insert(rl);
    return rl;
}

void ComponentScript::RenderObjectDelete(RenderableObject* obj){
    std::set<RenderableObject*>::iterator ri=mRenderableObjects.find(obj);
    if(ri==mRenderableObjects.end()){return;}

    RenderableObject* ro=(*ri);
    mRenderableObjects.erase(ri);
    delete ro;
}*/


void ComponentScript::ExposeProperties (std::map<std::string, int>& table){
    LuaRef LEngine= getGlobal(lState, "LEngine");
    LuaRef properties= LEngine["InitializationTable"];

    std::map<std::string, int>::iterator i=table.begin();
    for(; i!=table.end(); i++){
        properties[(*i).first]=(*i).second;
    }
}
void ComponentScript::ExposeProperties (std::map<std::string, bool>& table){
    LuaRef LEngine= getGlobal(lState, "LEngine");
    LuaRef properties= LEngine["InitializationTable"];

    std::map<std::string, bool>::iterator i=table.begin();
    for(; i!=table.end(); i++){
        properties[(*i).first]=(*i).second;
    }
}
void ComponentScript::ExposeProperties (std::map<std::string, double>& table){
    LuaRef LEngine= getGlobal(lState, "LEngine");
    LuaRef properties= LEngine["InitializationTable"];

    std::map<std::string, double>::iterator i=table.begin();
    for(; i!=table.end(); i++){
        properties[(*i).first]=(*i).second;
    }
}
void ComponentScript::ExposeProperties (std::map<std::string, std::string>& table){
    LuaRef LEngine= getGlobal(lState, "LEngine");
    LuaRef properties= LEngine["InitializationTable"];

    std::map<std::string, std::string>::iterator i=table.begin();
    for(; i!=table.end(); i++){
        properties[(*i).first]=(*i).second;
    }
}

void ComponentScript::HandleEvent(const Event* event){
}

void ComponentScript::Update(){
    RunFunction("Update");
}

LuaRef ComponentScript::GetEntityInterface(){
    return scriptPointer["EntityInterface"];
}

void ComponentScript::RunFunction(const std::string& fname){
    try{
        //get function from instance table
        LuaRef fN = scriptPointer[fname.c_str()];
        if (fN.isNil()) {
            ErrorLog::WriteToFile("Tried to run lua function; " + fname, logFileName);
            ErrorLog::WriteToFile("Function not found in script with EID " + mEntityID, logFileName);
            return;
        }
        if (!fN.isFunction()) {
            ErrorLog::WriteToFile("Tried to run lua function; " + fname, logFileName);
            ErrorLog::WriteToFile("Passed name is not function in script with EID " + mEntityID, logFileName);
            return;
        }
        fN();
    }
    catch (LuaException const& e){
        std::stringstream ss;
        ss << "Lua Exception: " << e.what ()
        << "\nScript Name is " << scriptName;
        ErrorLog::WriteToFile(ss.str(), logFileName);
    }
}

//////////////////////////
//ComponentScriptManager//
//////////////////////////

ComponentScriptManager::ComponentScriptManager(lua_State* state, LuaInterface* interface)
    : BaseComponentManager("LOG_COMP_SCRIPT"), lState(state), lInterface(interface){

}

void ComponentScriptManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentScript* script=new ComponentScript(id, logFileName);
    componentList[id]=script;
}

#endif
