#include "CompScript.h"

#include "../Kernel.h"
#include "CompCollision.h"
#include <sstream>

using namespace luabridge;

const std::string ComponentScript::entityDeletedDescription = "[ENTITY_DELETED]";

void RunInit(){
    //Push function on the stack
}

ComponentScript::ComponentScript(EID id, lua_State* state, EventDispatcher* ed, LuaInterface* interface, RenderManager* rm, ComponentScriptManager* manager)
: BaseComponent(id, manager), lState(state), lInterface(interface), scriptPointer(state), dependencyRenderManager(rm), dependencyEventDispatcher(ed)
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
void ComponentScript::EventLuaRemoveAllObservers(){
	mEventLuaObservers.clear();
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
                LOG_ERROR("OnKeyDown not found in script with EID " + (mEntityID));
                return;
            }
            if (!fKDown.isFunction()) {
                LOG_ERROR("OnKeyDown not function in script with EID " + (mEntityID));
                return;
            }
			std::string key = event->description;
            fKDown(key);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            LOG_INFO(ss.str());
        }
    }

    else if(event->message== Event::MSG::KEYUP){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fKUp = scriptPointer["OnKeyUp"];
            if (fKUp.isNil()) {
                LOG_ERROR("OnKeyUp not found in script with EID " + (mEntityID));
                return;
            }
            if (!fKUp.isFunction()) {
                LOG_ERROR("OnKeyUp not found in script with EID " + (mEntityID));
                return;
            }
			std::string key = event->description;
            fKUp(key);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "In OnKeyUp event in script with EID " << mEntityID << "\n"
            << "   ...Lua Exception " << e.what();
            LOG_ERROR(ss.str());
        }
    }

    else if(event->message== Event::MSG::COLLISION_ENTITY){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fEC = scriptPointer["OnEntityCollision"];
            if (fEC.isNil()) {
                LOG_ERROR("OnEntityCollision not found in script with EID " + (mEntityID));
                return;
            }
            if (!fEC.isFunction()) {
                LOG_ERROR("OnEntityCollision not function in script with EID " + (mEntityID));
                return;
            } 
			auto packet = EColPacket::ExtraDataDefinition::GetExtraData(event);
            fEC(event->sender, *packet); //pass other entity's id and the collision packet
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            LOG_ERROR(ss.str());
        }
    }

    else if(event->message== Event::MSG::COLLISION_TILE){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fTC = scriptPointer["OnTileCollision"];
            if (fTC.isNil()) {
                std::stringstream ss;
                ss << "OnTileCollision not found in script with EID " << (mEntityID) << " Name is: " << scriptName;
                LOG_ERROR(ss.str());
                return;
            }
            if (!fTC.isFunction()) {
                LOG_ERROR("OnTileCollision not found in script with EID " + (mEntityID));
                return;
            }
			auto packet = TColPacket::ExtraDataDefinition::GetExtraData(event);
            fTC(*packet);
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            LOG_ERROR(ss.str());
        }
    }
    else if(event->message== Event::MSG::LUA_EVENT){
		if(scriptPointer.isNil()){return;}
        try{
            LuaRef fTC =  scriptPointer["OnLuaEvent"];
            std::stringstream ss;
            ss << "OnLuaEvent not found in script with EID " << (mEntityID) << " Name is: " << scriptName;
            if (fTC.isNil()) {
                LOG_ERROR(ss.str());
                return;
            }
            if (!fTC.isFunction()) {
                LOG_ERROR("OnLuaEvent not found in script with EID " + (mEntityID));
                return;
            }

            fTC(event->sender, event->description); //pass other entity's id and description
        }
        catch (LuaException const& e){
            std::stringstream ss;
            ss << "Lua Exception: " << e.what ();
            LOG_ERROR(ss.str());
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
            LOG_ERROR("Tried to run lua function; " + fname);
            LOG_ERROR("Function not found in script with EID " + (mEntityID));
            return;
        }
        if (!fN.isFunction()) {
            LOG_ERROR("Tried to run lua function; " + fname);
            LOG_ERROR("Passed name is not function in script with EID " + (mEntityID));
            return;
        }
        fN();
		
    }
    catch (const LuaException& e){
        std::stringstream ss;
        ss << "Lua Exception: " << e.what ()
        << "\nScript Name is " << scriptName;
        LOG_ERROR(ss.str());

		#ifdef DEBUG_MODE
			std::cout << ss.str() << std::endl;
			throw e;
		#endif	
    }
	
}

//////////////////////////
//ComponentScriptManager//
//////////////////////////

ComponentScriptManager::~ComponentScriptManager(){
	/**
	When a state is being deleted, component scripts send out events, which the 
	event manager recieves, which is sent out to component managers which
	may have been deleted

	to prevent this, all listeners are removed before deletion
	*/

	for(auto i = componentList.begin(); i != componentList.end(); i++){
		((ComponentScript*)(i->second.get()))->EventLuaRemoveAllObservers();
	}
}

ComponentScriptManager::ComponentScriptManager(lua_State* state, LuaInterface* interface, EventDispatcher* e)
    : BaseComponentManager_Impl(e), lState(state), lInterface(interface){

}

std::unique_ptr<ComponentScript> ComponentScriptManager::ConstructComponent (EID id, ComponentScript* parent){
    auto script = make_unique<ComponentScript>(id, lState, eventDispatcher, lInterface, dependencyRenderManager, this);

    return std::move(script);
}

void ComponentScriptManager::SetDependencies(RenderManager* rm){
	dependencyRenderManager = rm;
}
