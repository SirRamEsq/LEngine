#ifndef L_ENGINE_LUAINTERFACE
#define L_ENGINE_LUAINTERFACE

// Engine headers
#include "Defines.h"
#include "LuaInclude.h"

#include "Components/CompCollision.h"
#include "Components/CompCamera.h"
#include "Components/CompParticle.h"
#include "Components/CompPosition.h"
#include "Components/CompSprite.h"
#include "Components/CompLight.h"

#include "Resources/RSC_Map.h"
#include "Resources/RSC_Script.h"

#include <unordered_set>
#include <functional>

// Forward declares
class GameState;
class ComponentPosition;
class ComponentCollision;
class ComponentParticle;
class ComponentScriptManager;
class ComponentScript;
class GS_Script;

std::string GetStringFromLuaStack(lua_State *state, int stackIndex);

std::unordered_map<std::string, luabridge::LuaRef> GetKeyValueMap(
    const luabridge::LuaRef &table);

// Forwared Declare
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
                 /*optional args*/ const TiledProperties *tiledProperties,
                 luabridge::LuaRef *initTable);

  std::string ExecuteString(const std::string &code);
  ///////////
  // General//
  ///////////
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
  const RSC_Sprite *LoadSpriteResource(const std::string &sprPath);

  Vec2 GetMousePosition();
  float GetMouseWheel();
  bool GetMouseButtonLeft();
  bool GetMouseButtonRight();
  bool GetMouseButtonMiddle();

  std::string GetTileProperty(const TiledTileLayer *layer, GID id,
                              const std::string &property);
  GS_Script *GetCurrentGameState();

  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      RSC_Map *m, const std::string &name, luabridge::LuaRef value);

  /// Implements a break-point in dbug mode
  void LUA_BREAK(EID id);

  //////////////
  // Components//
  //////////////
  ComponentPosition *GetPositionComponent(const EID &id);
  ComponentSprite *GetSpriteComponent(const EID &id);
  ComponentCollision *GetCollisionComponent(const EID &id);
  ComponentParticle *GetParticleComponent(const EID &id);
  ComponentCamera *GetCameraComponent(const EID &id);
  ComponentLight *GetLightComponent(const EID &id);
  ComponentScript *GetScriptComponent(const EID &id);

  bool HasPositionComponent(const EID &id);
  bool HasSpriteComponent(const EID &id);
  bool HasCollisionComponent(const EID &id);
  bool HasParticleComponent(const EID &id);
  bool HasCameraComponent(const EID &id);
  bool HasScriptComponent(const EID &id);
  bool HasLightComponent(const EID &id);

  ////////////
  // Entities//
  ////////////
  luabridge::LuaRef EntityGetInterface(const EID &id);

  /////////////
  // Rendering//
  /////////////
  RenderLine *RenderObjectLine(EID selfID, int x, int y, int xx, int yy);
  void RenderObjectDelete(EID selfID, RenderableObject *obj);

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

  /// Will push a new GS_Script state onto the stack with a script
  GS_Script *PushState(const std::string &scriptPath);
  /// Will pop current state off the stack
  void PopState();

  /// Will swap state with a new GS_Script
  void SwapState(const std::string &scriptPath);
  /// Will load map at specified path, then later call the callback when the new
  /// map is loaded
  bool LoadMap(const std::string &mapPath, unsigned int entranceID,
               luabridge::LuaRef callback);
  /// Sets a key
  void RemapInputToNextKeyPress(const std::string &key);
  /// Gets screen resolution
  Vec2 GetResolution();
  Vec2 GetVirtualResolution();
  void SimulateKeyPress(const std::string &keyName);
  void SimulateKeyRelease(const std::string &keyName);
  bool RecordKeysBegin();
  bool RecordKeysEnd();

  EntityManager *GetEntityManager() const;
  ComponentLightManager *GetLightManager() const;
  ComponentSpriteManager *GetSpriteManager() const;
  ComponentCameraManager *GetCameraManager() const;
  ComponentPositionManager *GetPositionManager() const;
  ComponentParticleManager *GetParticleManager() const;
  ComponentCollisionManager *GetCollisionManager() const;
  ComponentScriptManager *GetScriptManager() const;

  static const std::string LUA_52_INTERFACE_ENV_TABLE;
  static const std::string DEBUG_LOG;
  static const std::string BASE_SCRIPT_NAME;
  static const std::string TYPE_DIR;

  lua_State *GetState() { return lState; }

 protected:
  /// Creates 'CPP' table in the global table
  void ExposeCPP();
  /// Acts as require for lua
  luabridge::LuaRef ModuleLoad(const std::string &moduleName);
  std::map<std::string, int> mLoadedModules;

 private:
  void PrintToConsole(const std::string& str);
  void DebugTraceLine(const std::string& file, int line);
  /// Will Get the loaded lua function associated with the passed lua script
  int LookupFunction(const RSC_Script *script);
  /// Will process mEntitiesToObserve
  void ProcessObservers();

  // Will return a reference to the table representing the entity
  luabridge::LuaRef GetEntityTable(EID id);

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
