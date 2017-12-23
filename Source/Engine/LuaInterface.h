#ifndef L_ENGINE_LUAINTERFACE
#define L_ENGINE_LUAINTERFACE

// Engine headers
#include "Defines.h"
// Lua headers
extern "C" {
#include "lua5.2/lauxlib.h"
#include "lua5.2/lua.h"
#include "lua5.2/lualib.h"
}
#include "LuaCallback.h"
#include <LuaBridge.h>

#include "Components/CompCollision.h"
#include "Components/CompParticle.h"
#include "Components/CompPosition.h"
#include "Components/CompSprite.h"
#include "Components/CompLight.h"
#include "LuaBridgeVectorWrapper.h"

#include "Resources/RSC_Map.h"
#include "Resources/RSC_Script.h"

#include <unordered_set>

// Forward declares
class GameState;
class ComponentPosition;
class ComponentCollision;
class ComponentParticle;
class GS_Script;

std::unordered_map<std::string, luabridge::LuaRef> GetKeyValueMap(
    const luabridge::LuaRef &table);

/// This struct is passed to the GameState to instantiate a new entity from lua
struct EntityCreationPacket {
  /**
   * Called from lua scripts
   * \param scriptName Name of Script to run for the entity
   * \param pos Starting position of entity
   * \param depth Depth of entity
   * \param parent Entity's parent
   * \param name Name of entity
   * \param type Types that an entity uses
   * \param propertyTable Extra lua properties
   */
  EntityCreationPacket(std::vector<std::string> scripts, Coord2df pos,
                       MAP_DEPTH depth, EID parent, const std::string &name,
                       luabridge::LuaRef propertyTable);

  EID mParent;
  EID mNewEID;
  MAP_DEPTH mDepth;
  Coord2df mPos;
  std::vector<std::string> mScriptNames;
  std::string mEntityName;
  luabridge::LuaRef mPropertyTable;
};

//Forwared Declare
class GS_Test;

class LuaInterface {
  friend GameState;
  friend GS_Test;

 public:
  typedef void (*ErrorCallback)(EID id, const std::string &errorMessage);

  // Initializes and exposes the C++ functionality to the luaState
  LuaInterface(GameState *state);
  ~LuaInterface();

  bool RunScript(EID id, std::vector<const RSC_Script *> scripts,
                 MAP_DEPTH depth, EID parent, const std::string &name,
                 /*optional args*/ const TiledObject *obj,
                 luabridge::LuaRef *initTable);

  ///////////
  // General//
  ///////////
  LuaCallback CreateCallback(luabridge::LuaRef cb);
  void ListenForInput(EID id, const std::string &inputName);
  void LogFatal(EID id, const std::string &error);
  void LogError(EID id, const std::string &error);
  void LogWarn(EID id, const std::string &error);
  void LogInfo(EID id, const std::string &error);
  void LogDebug(EID id, const std::string &error);
  void LogTrace(EID id, const std::string &error);
  std::string GenerateLogEntry(EID id, const std::string &error);
  // sends event to audio subsystem
  void PlaySound(const std::string &sndName, int volume);
  void PlayMusic(const std::string &musName, int volume, int loops);
  const RSC_Sprite *LoadSprite(const std::string &sprPath);

  Coord2df GetMousePosition();
  float GetMouseWheel();
  bool GetMouseButtonLeft();
  bool GetMouseButtonRight();
  bool GetMouseButtonMiddle();

  std::string GetTileProperty(const TiledTileLayer *layer, GID id,
                              const std::string &property);
  GS_Script *GetCurrentGameState();

  void DeleteLayer(TiledLayerGeneric *layer);
  LB_VEC_WRAPPER<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, luabridge::LuaRef value);

  //////////////
  // Components//
  //////////////
  ComponentPosition *GetPositionComponent(const EID &id);
  ComponentSprite *GetSpriteComponent(const EID &id);
  ComponentCollision *GetCollisionComponent(const EID &id);
  ComponentParticle *GetParticleComponent(const EID &id);
  ComponentCamera *GetCameraComponent(const EID &id);
  ComponentLight *GetLightComponent(const EID &id);

  bool HasPositionComponent(const EID &id);
  bool HasSpriteComponent(const EID &id);
  bool HasCollisionComponent(const EID &id);
  bool HasParticleComponent(const EID &id);
  bool HasCameraComponent(const EID &id);
  bool HasLightComponent(const EID &id);

  ////////////
  // Entities//
  ////////////
  const std::vector<EID> *EntityGetFromName(const std::string &name);
  luabridge::LuaRef EntityGetInterface(const EID &id);
  Coord2df EntityGetPositionWorld(EID entity);
  Coord2df EntityGetMovement(EID entity);

  EID EntityNew(std::string name, int x, int y, MAP_DEPTH depth, EID parent,
                luabridge::LuaRef scripts, luabridge::LuaRef propertyTable);
  void EntityDelete(EID entity);

  /////////////
  // Rendering//
  /////////////
  RenderLine *RenderObjectLine(EID selfID, int x, int y, int xx, int yy);
  void RenderObjectDelete(EID selfID, RenderableObject *obj);

  //////////
  // Events//
  //////////
  /**
   * Listen to all events from a certain entity
   * will delay adding the observer till the start of the next frame
   */
  void EventLuaObserveEntity(EID listenerID, EID senderID);
  /**
   * Broadcast an event to all entities
   */
  void EventLuaBroadcastEvent(EID senderID, const std::string &event);
  /**
   * Send event to all observers
   */
  void EventLuaSendToObservers(EID senderID, const std::string &event);
  /**
   * Directly send event to specified EID
   */
  void EventLuaSendEvent(EID senderID, EID recieverID,
                         const std::string &event);

  ///////////
  // Handles//
  ///////////
  RSC_Map *GetMap();

  ///////////
  // Testing//
  ///////////
  void SetErrorCallbackFunction(ErrorCallback func);

  /////////
  // OTHER//
  /////////
  /// Set parent for all component managers defined in the EntityManager
  void SetParent(EID child, EID parent);

  /// update once per frame
  void Update();

  /// Will push a new GS_Script state onto the stack with a script
  GS_Script *PushState(const std::string &scriptPath);
  /// Will pop current state off the stack
  void PopState();

  /// Will swap state with a new GS_Script
  void SwapState(const std::string &scriptPath);
  /// Will load map at specified path
  bool LoadMap(const std::string &mapPath, unsigned int entranceID);
  /// Sets a key
  void RemapInputToNextKeyPress(const std::string &key);
  /// Gets screen resolution
  Coord2df GetResolution();
  Coord2df GetVirtualResolution();
  void SimulateKeyPress(const std::string &keyName);
  void SimulateKeyRelease(const std::string &keyName);
  bool RecordKeysBegin();
  bool RecordKeysEnd();

  void SetAmbientLight(float r, float g, float b);

  static const std::string LUA_52_INTERFACE_ENV_TABLE;
  static const std::string DEBUG_LOG;
  static const std::string BASE_SCRIPT_NAME;
  static const std::string TYPE_DIR;

  lua_State *GetState() { return lState; }
 protected:
  /// Creates 'CPP' table in the global table
  void ExposeCPP();

 private:
  /// Will Get the loaded lua function associated with the passed lua script
  int LookupFunction(const RSC_Script *script);
  /// Will process mEntitiesToObserve
  void ProcessObservers();

  ErrorCallback errorCallbackFunction;
  /// All entities derive from this
  const RSC_Script *baseScript;
  int baseLuaClass;

  // RunScript Helper Functions
  int RunScriptLoadFromChunk(const RSC_Script *script);
  int RunScriptGetChunk(const RSC_Script *script);

  /// used to access component managers
  GameState *parentState;

  /// this class is the owner of the lua state for the current gamestate
  lua_State *lState;

  /// Map of scripts to (a reference of) their lua functions that return a new
  /// instance.
  std::map<const RSC_Script *, int> mFunctionLookup;

  /// Contains entities that will be setup to observe each other next frame
  std::map<EID, std::unordered_set<EID> > mEntitiesToObserve;
};

#endif  // L_ENGINE_LUAINTERFACE
