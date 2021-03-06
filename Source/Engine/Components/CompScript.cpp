#include "CompScript.h"

#include "../Kernel.h"
#include "CompCollision.h"
#include <sstream>
#include "../TiledProperties.h"

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
  SendEvent(entityDeletedDescription);
}

bool ComponentScript::UsesScript(const std::string &scriptName) {
  auto script = K_ScriptMan.GetItem(scriptName);
  if (script == NULL) {
    return false;
  }
  return UsesScript(script);
}
bool ComponentScript::UsesScript(const RSC_Script *script) {
  for (auto i = mScriptsUsed.begin(); i != mScriptsUsed.end(); i++) {
    if (script == *i) {
      return true;
    }
  }
  return false;
}

void ComponentScript::SetScriptPointerOnce(
    luabridge::LuaRef lp, const std::vector<const RSC_Script *> *scripts) {
  mScriptsUsed = *scripts;
  if (!scriptPointer.isNil()) {
    std::stringstream ss;
    ss << "LuaPointer already set in script " << scriptName << " with EID "
       << mEntityID;
    throw LEngineException(ss.str());
  }
  scriptPointer = lp;
}

void ComponentScript::BroadcastEvent(const std::string &event) {
  Event eventStructure(GetEID(), EID_ALLOBJS, Event::MSG::LUA_EVENT, event);
  dependencyEventDispatcher->DispatchEvent(eventStructure);
}

void ComponentScript::SendEvent(const std::string &event) {
  Event e(GetEID(), 0, Event::MSG::LUA_EVENT, event);
  for (auto i : mEventLuaObservers) {
    e.reciever = i;
    dependencyEventDispatcher->DispatchEvent(e);
  }
}

void ComponentScript::AddObserver(EID id) { mEventLuaObservers.insert(id); }
void ComponentScript::RemoveObserver(EID id) { mEventLuaObservers.erase(id); }
void ComponentScript::RemoveAllObservers() { mEventLuaObservers.clear(); }

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
    RemoveObserver(event->sender);
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
    }
    /*	else if (event->message == Event::MSG::COLLISION_TILE) {
          auto func = GetFunction("OnTileCollision");
          if (not func.isNil()) {
            auto packet = TColPacket::ExtraDataDefinition::GetExtraData(event);
            func(*packet);
          }
        }*/
    else if (event->message == Event::MSG::LUA_EVENT) {
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
    try {
      func();
    } catch (LuaException const &e) {
      std::stringstream ss;
      ss << "Error Running function '" << fname << "'"
         << "\nIn script with name '" << scriptName << "'"
         << "\nwhat()" << e.what();
      LOG_ERROR(ss.str());
    }
  }
}

std::string ComponentScript::GetScriptName() const { return scriptName; }

luabridge::LuaRef ComponentScript::GetInitializationTable() {
  /*
  if (!scriptPointer.isNil()){
    auto LEngineTable = scriptPointer["LEngine"];
    if (LEngineTable.isNil()){
      return LEngineTable;
    }
  }
  */
  // luabridge::LuaRef lengineData = scriptPointer["LEngineData"];
  // return lengineData["InitializationTable"];
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

  for (auto i = mComponentList.begin(); i != mComponentList.end(); i++) {
    ((ComponentScript *)(i->second.get()))->RemoveAllObservers();
  }
}

ComponentScriptManager::ComponentScriptManager(lua_State *state,
                                               LuaInterface *interface,
                                               EventDispatcher *e)
    : BaseComponentManager_Impl(e), lState(state), lInterface(interface) {}

std::unique_ptr<ComponentScript> ComponentScriptManager::ConstructComponent(
    EID id, ComponentScript *parent) {
  auto script = std::make_unique<ComponentScript>(
      id, lState, mEventDispatcher, lInterface, dependencyRenderManager, this);

  return std::move(script);
}

void ComponentScriptManager::SetDependencies(RenderManager *rm) {
  dependencyRenderManager = rm;
}

void ComponentScriptManager::CreateEntity(EID id,
                                          std::vector<std::string> scripts,
                                          luabridge::LuaRef propertyTable) {
  std::vector<const RSC_Script *> rscScripts;
  for (auto i : scripts) {
    auto rsc = K_ScriptMan.GetLoadItem(i);
    if (rsc != NULL) {
      rscScripts.push_back(rsc);
    }
  }
  lInterface->RunScript(id, rscScripts, NULL, &propertyTable);
}

void ComponentScriptManager::CreateEntityPrefab(
    EID id, std::string prefabName, luabridge::LuaRef propertyTable) {
  prefabName = tiledProperties::object::PREFAB_PREFIX + prefabName;
  auto prefab = K_PrefabMan.GetLoadItem(prefabName, prefabName);
  if (prefab == NULL) {
    std::stringstream ss;
    ss << "Prefab named '" << prefabName << "' does not exist";
    LOG_ERROR(ss.str());
    return;
  }
  std::vector<const RSC_Script *> scripts;
  for (auto i : prefab->mScripts) {
    auto script = K_ScriptMan.GetLoadItem(i, i);
    if (script != NULL) {
      scripts.push_back(script);
    }
  }
  lInterface->RunScript(id, scripts, &prefab->mProperties, &propertyTable);
}

void ComponentScriptManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")
      .beginClass<ComponentScriptManager>("ComponentScriptManager")
      .addFunction("CreateEntity", &ComponentScriptManager::CreateEntity)
      .addFunction("CreateEntityPrefab",
                   &ComponentScriptManager::CreateEntityPrefab)
      .endClass()

      .beginClass<ComponentScript>("ComponentScript")
      .addFunction("BroadcastEvent", &ComponentScript::BroadcastEvent)
      .addFunction("SendEvent", &ComponentScript::SendEvent)
      .addFunction("AddObserver", &ComponentScript::AddObserver)
      .addFunction("RemoveObserver", &ComponentScript::RemoveObserver)
      .addFunction("RemoveAllObservers", &ComponentScript::RemoveAllObservers)
      .endClass()

      .endNamespace();
}
