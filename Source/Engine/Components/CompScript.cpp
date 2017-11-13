#include "CompScript.h"

#include "../Kernel.h"
#include "CompCollision.h"
#include <sstream>

using namespace luabridge;

const std::string ComponentScript::entityDeletedDescription =
    "[ENTITY_DELETED]";

void RunInit() {
  // Push function on the stack
}

ComponentScript::ComponentScript(EID id, lua_State *state, EventDispatcher *ed,
                                 LuaInterface *interface, RenderManager *rm,
                                 ComponentScriptManager *manager)
    : BaseComponent(id, manager),
      lState(state),
      lInterface(interface),
      scriptPointer(state),
      dependencyRenderManager(rm),
      dependencyEventDispatcher(ed) {}

ComponentScript::~ComponentScript() {
  // Let all observers know that this entity has been deleted
  EventLuaSendToObservers(entityDeletedDescription);
}

void ComponentScript::SetScriptPointerOnce(luabridge::LuaRef lp) {
  if (!scriptPointer.isNil()) {
    std::stringstream ss;
    ss << "LuaPointer already set in script " << scriptName << " with EID "
       << mEntityID;
    throw LEngineException(ss.str());
  }
  scriptPointer = lp;
}

void ComponentScript::EventLuaBroadcastEvent(const std::string &event) {
  Event eventStructure(GetEID(), EID_ALLOBJS, Event::MSG::LUA_EVENT, event);
  dependencyEventDispatcher->DispatchEvent(eventStructure);
}

void ComponentScript::EventLuaSendToObservers(const std::string &event) {
  Event e(GetEID(), 0, Event::MSG::LUA_EVENT, event);
  for (auto it = mEventLuaObservers.begin(); it != mEventLuaObservers.end();
       it++) {
    e.reciever = it->first;
    // Formerly: it->second->HandleEvent(e);
    dependencyEventDispatcher->DispatchEvent(e);
  }
}

bool ComponentScript::EventLuaAddObserver(ComponentScript *script) {
  if (mEventLuaObservers.find(script->GetEID()) == mEventLuaObservers.end()) {
    mEventLuaObservers[script->GetEID()] = script;
    return true;
  }
  return false;
}
bool ComponentScript::EventLuaRemoveObserver(EID id) {
  if (mEventLuaObservers.find(id) != mEventLuaObservers.end()) {
    mEventLuaObservers.erase(id);
    return true;
  }
  return false;
}
void ComponentScript::EventLuaRemoveAllObservers() {
  mEventLuaObservers.clear();
}

RenderText *ComponentScript::RenderObjectText(int x, int y,
                                              const std::string &text,
                                              bool abss) {
  RenderText *rt = new RenderText(dependencyRenderManager, x, y, text, abss);
  mRenderableObjects.insert(rt);
  return rt;
}

RenderLine *ComponentScript::RenderObjectLine(int x, int y, int xx, int yy) {
  RenderLine *rl = new RenderLine(dependencyRenderManager, x, y, xx, yy);
  mRenderableObjects.insert(rl);
  return rl;
}

void ComponentScript::RenderObjectDelete(RenderableObject *obj) {
  std::set<RenderableObject *>::iterator ri = mRenderableObjects.find(obj);
  if (ri == mRenderableObjects.end()) {
    return;
  }

  RenderableObject *ro = (*ri);
  mRenderableObjects.erase(ri);
  delete ro;
}

void ComponentScript::ExposeProperties(std::map<std::string, int> &table) {
  LuaRef LEngine = getGlobal(lState, "LEngine");
  LuaRef properties = LEngine["InitializationTable"];

  std::map<std::string, int>::iterator i = table.begin();
  for (; i != table.end(); i++) {
    properties[(*i).first] = (*i).second;
  }
}
void ComponentScript::ExposeProperties(std::map<std::string, bool> &table) {
  LuaRef LEngine = getGlobal(lState, "LEngine");
  LuaRef properties = LEngine["InitializationTable"];

  std::map<std::string, bool>::iterator i = table.begin();
  for (; i != table.end(); i++) {
    properties[(*i).first] = (*i).second;
  }
}
void ComponentScript::ExposeProperties(std::map<std::string, double> &table) {
  LuaRef LEngine = getGlobal(lState, "LEngine");
  LuaRef properties = LEngine["InitializationTable"];

  std::map<std::string, double>::iterator i = table.begin();
  for (; i != table.end(); i++) {
    properties[(*i).first] = (*i).second;
  }
}
void ComponentScript::ExposeProperties(
    std::map<std::string, std::string> &table) {
  LuaRef LEngine = getGlobal(lState, "LEngine");
  LuaRef properties = LEngine["InitializationTable"];

  std::map<std::string, std::string>::iterator i = table.begin();
  for (; i != table.end(); i++) {
    properties[(*i).first] = (*i).second;
  }
}

void ComponentScript::HandleEvent(const Event *event) {
  // This event doesn't need the script pointer to be valid
  if (event->message == Event::MSG::ENTITY_DELETED) {
    EventLuaRemoveObserver(event->sender);
  }

  // The following events need the script pointer to be valid
  if (scriptPointer.isNil()) {
    return;
  }

  try {
    if (event->message == Event::MSG::KEYDOWN) {
      auto func = GetFunction("OnKeyDown");
      if (not func.isNil()) {
        auto key = event->description;
        func(key);
      }
    } else if (event->message == Event::MSG::KEYUP) {
      auto func = GetFunction("OnKeyUp");
      if (not func.isNil()) {
        auto key = event->description;
        func(key);
      }
    } else if (event->message == Event::MSG::COLLISION_ENTITY) {
      auto func = GetFunction("OnEntityCollision");
      if (not func.isNil()) {
        auto packet = EColPacket::ExtraDataDefinition::GetExtraData(event);
        func(event->sender, *packet);
      }
    } else if (event->message == Event::MSG::COLLISION_TILE) {
      auto func = GetFunction("OnTileCollision");
      if (not func.isNil()) {
        auto packet = TColPacket::ExtraDataDefinition::GetExtraData(event);
        func(*packet);
      }
    } else if (event->message == Event::MSG::LUA_EVENT) {
      auto func = GetFunction("OnLuaEvent");
      if (not func.isNil()) {
        func(event->sender, event->description);
      }
    }
  } catch (LuaException const &e) {
    std::stringstream ss;
    ss << "Lua Exception: " << e.what();
    LOG_ERROR(ss.str());
  }
}

void ComponentScript::Update() { RunFunction("Update"); }

LuaRef ComponentScript::GetEntityInterface() {
  return scriptPointer["EntityInterface"];
}

LuaRef ComponentScript::GetFunction(const std::string &fname) {
  LuaRef nil(GetState());
  try {
    // get function from instance table
    LuaRef fN = scriptPointer[fname.c_str()];
    if ((fN.isNil()) or (!fN.isFunction())) {
      std::stringstream ss;
      ss << "Failed to find lua function '" << fname << "'" << std::endl
         << "    Name '" << scriptName << "' | "
         << "EID [" << mEntityID << "]";
      LOG_WARN(ss.str());
      return nil;
    }
    return fN;

  } catch (const LuaException &e) {
    std::stringstream ss;
    ss << "Lua Exception: " << e.what() << "\nScript Name is " << scriptName;
    LOG_ERROR(ss.str());

#ifdef DEBUG_MODE
    std::cout << ss.str() << std::endl;
    throw e;
#endif
  }

  return nil;
}

void ComponentScript::RunFunction(const std::string &fname) {
  auto func = GetFunction(fname);
  if (!func.isNil()) {
    func();
  }
}

//////////////////////////
// ComponentScriptManager//
//////////////////////////

ComponentScriptManager::~ComponentScriptManager() {
  /**
  When a state is being deleted, component scripts send out events, which the
  event manager recieves, which is sent out to component managers which
  may have been deleted

  to prevent this, all listeners are removed before deletion
  */

  for (auto i = componentList.begin(); i != componentList.end(); i++) {
    ((ComponentScript *)(i->second.get()))->EventLuaRemoveAllObservers();
  }
}

ComponentScriptManager::ComponentScriptManager(lua_State *state,
                                               LuaInterface *interface,
                                               EventDispatcher *e)
    : BaseComponentManager_Impl(e), lState(state), lInterface(interface) {}

std::unique_ptr<ComponentScript> ComponentScriptManager::ConstructComponent(
    EID id, ComponentScript *parent) {
  auto script = std::make_unique<ComponentScript>(
      id, lState, eventDispatcher, lInterface, dependencyRenderManager, this);

  return std::move(script);
}

void ComponentScriptManager::SetDependencies(RenderManager *rm) {
  dependencyRenderManager = rm;
}
