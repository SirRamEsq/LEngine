#include "CompScript.h"

#include "../Kernel.h"
#include "CompCollision.h"
#include <sstream>

using namespace luabridge;

const std::string ComponentScript::entityDeletedDescription = "[ENTITY_DELETED]";

void RunInit(){
    //Push function on the stack
}

ComponentScript::ComponentScript(EID id, lua_State* state, EventDispatcher* ed, LuaInterface* interface, RenderManager* rm, const std::string& logName)
: BaseComponent(id, logName), lState(state), lInterface(interface), scriptPointer(state), dependencyRenderManager(rm), dependencyEventDispatcher(ed)
{

}

ComponentScript::~ComponentScript(){
	//Let all observers know that this entity has been deleted
	EventLuaSendToObservers(entityDeletedDescription);
}

void ComponentScript::SetScriptPointerOnce(luabridge::LuaRef lp){
    if(!scriptPointer.isNil()){
        std::stringstream ss;
        ss << "LuaPointer already set in script " << scriptName << " with EID " << mEntityID;
        throw LEngineException(ss.str());
    }
    scriptPointer = lp;
}

void ComponentScript::EventLuaBroadcastEvent(const std::string& event){
	Event eventStructure (GetEID(), EID_ALLOBJS, Event::MSG::LUA_EVENT, event);
	dependencyEventDispatcher->DispatchEvent(eventStructure);
}

void ComponentScript::EventLuaSendToObservers(const std::string& event){
	Event e(GetEID(), 0, Event::MSG::LUA_EVENT, event);
    for(auto it=mEventLuaObservers.begin(); it!=mEventLuaObservers.end(); it++){
		e.reciever = it->first;
		//Formerly: it->second->HandleEvent(e);
		dependencyEventDispatcher->DispatchEvent(e);
    }
}

bool ComponentScript::EventLuaAddObserver    (ComponentScript* script){
    if(mEventLuaObservers.find(script->GetEID())==mEventLuaObservers.end()){
        mEventLuaObservers[script->GetEID()]=script;
		return true;
    }
	return false;
}
bool  ComponentScript::EventLuaRemoveObserver (EID id){
    if(mEventLuaObservers.find(id)!=mEventLuaObservers.end()){
        mEventLuaObservers.erase(id);
		return true;
    }
	return false;
}

RenderText* ComponentScript::RenderObjectText(int x, int y, const std::string& text, bool abss){
    RenderText* rt= new RenderText(dependencyRenderManager, x,y,text, abss);
    mRenderableObjects.insert(rt);
    return rt;
}

RenderLine* ComponentScript::RenderObjectLine(int x, int y, int xx, int yy){
    RenderLine* rl=new RenderLine(dependencyRenderManager, x,y,xx, yy);
    mRenderableObjects.insert(rl);
    return rl;
}

void ComponentScript::RenderObjectDelete(RenderableObject* obj){
    std::set<RenderableObject*>::iterator ri=mRenderableObjects.find(obj);
    if(ri==mRenderableObjects.end()){return;}

    RenderableObject* ro=(*ri);
    mRenderableObjects.erase(ri);
    delete ro;
}


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
    if(event->message== Event::MSG::KEYDOWN){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fKDown = scriptPointer["OnKeyDown"];
            if (fKDown.isNil()) {
                ErrorLog::WriteToFile("OnKeyDown not found in script with EID " + (mEntityID), logFileName);
                return;
            }
            if (!fKDown.isFunction()) {
                ErrorLog::WriteToFile("OnKeyDown not found in script with EID " + (mEntityID), logFileName);
                return;
            }
			std::string key = event->description;
            fKDown(key);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            ErrorLog::WriteToFile(ss.str(), logFileName);
        }
    }

    else if(event->message== Event::MSG::KEYUP){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fKUp = scriptPointer["OnKeyUp"];
            if (fKUp.isNil()) {
                ErrorLog::WriteToFile("OnKeyUp not found in script with EID " + (mEntityID), logFileName);
                return;
            }
            if (!fKUp.isFunction()) {
                ErrorLog::WriteToFile("OnKeyUp not found in script with EID " + (mEntityID), logFileName);
                return;
            }
			std::string key = event->description;
            fKUp(key);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "In OnKeyUp event in script with EID " << mEntityID << "\n"
            << "   ...Lua Exception " << e.what();
            ErrorLog::WriteToFile(ss.str(), logFileName);
        }
    }

    else if(event->message== Event::MSG::COLLISION_ENTITY){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fEC = scriptPointer["OnEntityCollision"];
            if (fEC.isNil()) {
                ErrorLog::WriteToFile("OnEntityCollision not found in script with EID " + (mEntityID), logFileName);
                return;
            }
            if (!fEC.isFunction()) {
                ErrorLog::WriteToFile("OnEntityCollision not found in script with EID " + (mEntityID), logFileName);
                return;
            } 
			auto packet = EColPacket::ExtraDataDefinition::GetExtraData(event);
            fEC(event->sender, *packet); //pass other entity's id and the collision packet
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            ErrorLog::WriteToFile(ss.str(), logFileName);
        }
    }

    else if(event->message== Event::MSG::COLLISION_TILE){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fTC = scriptPointer["OnTileCollision"];
            if (fTC.isNil()) {
                std::stringstream ss;
                ss << "OnTileCollision not found in script with EID " << (mEntityID) << " Name is: " << scriptName;
                ErrorLog::WriteToFile(ss.str(), logFileName);
                return;
            }
            if (!fTC.isFunction()) {
                ErrorLog::WriteToFile("OnTileCollision not found in script with EID " + (mEntityID), logFileName);
                return;
            }
			auto packet = TColPacket::ExtraDataDefinition::GetExtraData(event);
            fTC(*packet);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            ErrorLog::WriteToFile(ss.str(), logFileName);
        }
    }
    else if(event->message== Event::MSG::LUA_EVENT){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fTC =  scriptPointer["OnLuaEvent"];
            std::stringstream ss;
            ss << "OnLuaEvent not found in script with EID " << (mEntityID) << " Name is: " << scriptName;
            if (fTC.isNil()) {
                ErrorLog::WriteToFile(ss.str(), logFileName);
                return;
            }
            if (!fTC.isFunction()) {
                ErrorLog::WriteToFile("OnLuaEvent not found in script with EID " + (mEntityID), logFileName);
                return;
            }

            fTC(event->sender, event->description); //pass other entity's id and description
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            ErrorLog::WriteToFile(ss.str(), logFileName);
        }
    }
    else if(event->message== Event::MSG::ENTITY_DELETED){
        EventLuaRemoveObserver(event->sender);
	}
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
            ErrorLog::WriteToFile("Function not found in script with EID " + (mEntityID), logFileName);
            return;
        }
        if (!fN.isFunction()) {
            ErrorLog::WriteToFile("Tried to run lua function; " + fname, logFileName);
            ErrorLog::WriteToFile("Passed name is not function in script with EID " + (mEntityID), logFileName);
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

ComponentScriptManager::ComponentScriptManager(lua_State* state, LuaInterface* interface, EventDispatcher* e)
    : BaseComponentManager("LOG_COMP_SCRIPT", e), lState(state), lInterface(interface){

}

void ComponentScriptManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentScript* script=new ComponentScript(id, lState, eventDispatcher, lInterface, dependencyRenderManager, logFileName);
    componentList[id]=script;
}

void ComponentScriptManager::SetDependencies(RenderManager* rm){
	dependencyRenderManager = rm;
}
