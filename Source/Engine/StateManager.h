#ifndef L_STATE_MAN
#define L_STATE_MAN

#include <memory>
#include <stack>

#include "Components/CompCamera.h"
#include "Components/CompCollision.h"
#include "Components/CompInput.h"
#include "Components/CompLight.h"
#include "Components/CompParticle.h"
#include "Components/CompParticle.h"
#include "Components/CompPosition.h"
#include "Components/CompScript.h"
#include "Components/CompSprite.h"

#include "Event.h"
#include "LuaInterface.h"
#include "Resources/RSC_Map.h"

class LuaInterfaceB;
class GameStateManager_Impl;

class GameState {
  friend GameStateManager_Impl;
  friend EventDispatcher;
  friend LuaInterface;
  friend EntityManager;
  friend RenderManager;

  friend ComponentPositionManager;
  friend ComponentInputManager;
  friend ComponentScriptManager;
  friend ComponentSpriteManager;
  friend ComponentCollisionManager;
  friend ComponentParticleManager;
  friend ComponentCameraManager;
  friend ComponentLightManager;

 public:
  bool IsLuaState() const { return isLuaState; }
  GameState(GameStateManager *gsm);

  // Virtual destructor; enables derived classes to be fully deleted from a base
  // GameState pointer
  virtual ~GameState();

  RSC_Map *GetCurrentMap() { return mCurrentMap.get(); }

  RenderManager renderMan;
  /// Stores passed arguments till the start of the next frame, then loads the
  /// map
  void SetMapNextFrame(const RSC_Map *m, unsigned int entranceID);

  /// Creates a new Entity next frame
  EID CreateLuaEntity(std::unique_ptr<EntityCreationPacket> p);

 protected:
  virtual void Init(const RSC_Script *stateScript = NULL) = 0;

  /**
   * Function called when state is about to be removed from the stack
   * This function should not be treated like a destructor, the state beneath it
   * on the stack
   * may want to access its data after it is popped
   */
  virtual void Close() = 0;
  /// Function called when state is resumed after popping a state above it off
  /// the stack
  virtual void Resume() = 0;

  virtual void HandleEvent(const Event *event) = 0;
  virtual bool Update() = 0;
  virtual void Draw() = 0;
  virtual void SetDependencies();
  void UpdateComponentManagers();

  /// Get eid from data structure nameLookupEID
  const std::vector<EID>* GetEIDFromName(const std::string &name) const;

  LuaInterface luaInterface;
  EventDispatcher eventDispatcher;
  EntityManager entityMan;

  ComponentPositionManager comPosMan;
  ComponentInputManager comInputMan;
  ComponentScriptManager comScriptMan;
  ComponentSpriteManager comSpriteMan;
  ComponentCollisionManager comCollisionMan;
  ComponentParticleManager comParticleMan;
  ComponentCameraManager comCameraMan;
  ComponentLightManager comLightMan;

  GameStateManager *gameStateManager;

  std::shared_ptr<InputManager::KeyMapping> input;

  void DrawPreviousState();
  bool SetCurrentMap(const RSC_Map *m, unsigned int entranceID);

  const RSC_Map *nextMap;
  unsigned int nextMapEntrance;

  /// used by LuaInterface to determine if this state is a GS_Script
  bool isLuaState;

 private:
  void AddNameEIDLookup(const std::string &name, EID id);
  void SetMapHandleRenderableLayers(
      const std::map<MAP_DEPTH, TiledLayerGeneric *> &layers);
  // returns a data structure mapping tiled EIDS to engine EIDS
  std::map<EID, EID> SetMapCreateEntitiesFromLayers(
      const std::vector<std::unique_ptr<TiledObjectLayer>> &layers);
  std::vector<EID> SetMapGetEntitiesUsingEntrances(
      const std::vector<std::unique_ptr<TiledObjectLayer>> &layers);
  void SetMapLinkEntities(
      const std::vector<std::unique_ptr<TiledObjectLayer>> &layers,
      std::map<EID, EID> &tiledIDtoEntityID,
      const std::vector<EID> &objectsUsingEntrance);

  // is copy of what is stored in resource manager
  std::unique_ptr<RSC_Map> mCurrentMap;
  std::vector<std::unique_ptr<RenderableObjectWorld>> mCurrentMapTileLayers;
  /// This is used to lookup eids by entity name
  std::unordered_map<std::string, std::vector<EID>> mNameLookup;

  void CreateNewEntities();
  std::vector<std::unique_ptr<EntityCreationPacket>> mEntitiesToCreate;
};

// Interface
class GameStateManager {
  friend Kernel;
  friend EventDispatcher;
  friend EntityManager;
  friend GameState;

 public:
  GameStateManager(InputManager *input);
  virtual GameState *GetCurrentState() = 0;

  /**
   * Prepares to push State Next Frame
   * Sets internal nextFrameState and nextFrameScript
   * this class assumes ownership of the state
   * Can optionally pass a script to be run
   */
  virtual void PushState(const std::shared_ptr<GameState> &state,
                         const RSC_Script *script = NULL) = 0;

  /**
   * Pops current state and Pushes the new one at the beginning of the next
   * frame
   * Acts as a wrapper around 'PushState' that sets swapState to true
   */
  virtual void SwapState(const std::shared_ptr<GameState> &state,
                         const RSC_Script *script = NULL) = 0;

  /**
   * State is popped at the start of the next frame
   */
  virtual void PopState() = 0;
  virtual int StackSize() = 0;
  virtual bool IsEmpty() = 0;
  InputManager *inputManager;

 protected:
  virtual void Close() = 0;

  virtual void HandleEvent(const Event *event) = 0;
  virtual bool Update() = 0;
  virtual void Draw() = 0;

  virtual void DrawPreviousState(GameState *gs) = 0;
  virtual void PushNextState() = 0;
  virtual void PopTopState() = 0;
};

/**
 * Manages and owns all GameStates
 */
class GameStateManager_Impl : public GameStateManager {
  friend Kernel;
  friend EventDispatcher;
  friend EntityManager;
  friend GameState;

 public:
  GameStateManager_Impl(InputManager *input);
  GameState *GetCurrentState();

  void PushState(const std::shared_ptr<GameState> &state,
                 const RSC_Script *script = NULL);
  void SwapState(const std::shared_ptr<GameState> &state,
                 const RSC_Script *script = NULL);
  void PopState();
  int StackSize();
  bool IsEmpty();

 protected:
  void Close();

  void HandleEvent(const Event *event);
  bool Update();
  void Draw();

  void DrawPreviousState(GameState *gs);
  void PushNextState();
  void PopTopState();

 private:
  GameState *GetPreviousState(GameState *gs);
  GameState *mCurrentState;
  /// State to push next frame
  std::shared_ptr<GameState> nextFrameState;
  const RSC_Script *nextFrameStateScript;

  std::vector<std::shared_ptr<GameState>> mGameStates;

  /// Next key to remap
  std::string remapKey;

  /// Whether the next state to push should replace the current state
  bool swapState;
  bool popState;

  /*
   * Input Manager which is used to pass key mappings to each individual state
   * Handles Input and uses the current state's key mapping to send events
   */
};

#endif
