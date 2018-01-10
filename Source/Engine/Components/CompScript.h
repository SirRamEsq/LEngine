#ifndef L_SCRIPTCOMP
#define L_SCRIPTCOMP

#include "../BaseComponentManager.h"
#include "CompParticle.h"
#include "CompPosition.h"
#include "CompSprite.h"

#include "../Event.h"
#include "../LuaInterface.h"

#include "../Resources/RSC_Script.h"
#include "../Resources/RSC_Sound.h"
#include "../Resources/RSC_Sprite.h"

#include <set>

#include "../LuaInclude.h"

// Forward Declares
class Kernel;
class ComponentScriptManager;
class ComponentInput;
class ComponentScript;
class ComponentCollision;
class EColPacket;
class LuaInterface;

class ComponentScript : public BaseComponent {
  friend class ComponentScriptManager;
  friend class ComponentInput;
  friend class LuaInterface;
  friend class Kernel;
  friend class GameState;

 public:
  ComponentScript(EID id, lua_State *state, EventDispatcher *ed,
                  LuaInterface *interface, RenderManager *rm,
                  ComponentScriptManager *manager);
  ~ComponentScript();

  void HandleEvent(const Event *event);
  void Update();

  // interface defined inside script, for other scripts
  luabridge::LuaRef GetEntityInterface();

  void RunFunction(const std::string &fname);
  luabridge::LuaRef GetFunction(const std::string &fname);

  lua_State *GetState() { return lState; }

  void ExposeProperties(std::map<std::string, int> &table);
  void ExposeProperties(std::map<std::string, bool> &table);
  void ExposeProperties(std::map<std::string, double> &table);
  void ExposeProperties(std::map<std::string, std::string> &table);

  /**
   * Sends an event to all lua scrips
   */
  void EventLuaBroadcastEvent(const std::string &event);
  /**
   * Sends an event to all observers of this script
   */
  void EventLuaSendToObservers(const std::string &event);
  /**
   * Adds an observer
   * \returns TRUE if the observer was added and was NOT previously an observer
   */
  bool EventLuaAddObserver(ComponentScript *script);
  /**
   * Removes an observer
   * \returns TRUE if the observer was removed and WAS previously an observer
   */
  bool EventLuaRemoveObserver(EID id);

  void EventLuaRemoveAllObservers();

  ///*DEPRECIATED*
  RenderText *RenderObjectText(int x, int y, const std::string &text,
                               bool abss = true);
  ///*DEPRECIATED*
  RenderLine *RenderObjectLine(int x, int y, int xx, int yy);
  ///*DEPRECIATED*
  void RenderObjectDelete(RenderableObject *obj);

  std::string scriptName;
  std::string GetScriptName() const;
  static const std::string entityDeletedDescription;

  // Throws if script is already assigned
  void SetScriptPointerOnce(luabridge::LuaRef lp,
                            const std::vector<const RSC_Script *> *scripts);
  luabridge::LuaRef GetScriptPointer() { return scriptPointer; }

  bool UsesScript(const RSC_Script *script);
  bool UsesScript(const std::string &scriptName);

 protected:
  lua_State *lState;
  LuaInterface *lInterface;
  EventDispatcher *dependencyEventDispatcher;
  // pointer to this final table's location in the lua state
  luabridge::LuaRef scriptPointer;

 private:
  /// What scripts were used to generate the table pointed to by scriptPointer
  std::vector<const RSC_Script *> mScriptsUsed;
  // List of EIDs that are listening to this EID's events
  std::map<EID, ComponentScript *> mEventLuaObservers;
  // The renderable objects that this script is responsible for deleting
  std::set<RenderableObject *> mRenderableObjects;
  RenderManager *dependencyRenderManager;
};

struct TiledObject;
class ComponentScriptManager
    : public BaseComponentManager_Impl<ComponentScript> {
 public:
  ComponentScriptManager(lua_State *state, LuaInterface *interface,
                         EventDispatcher *e);
  ~ComponentScriptManager();

  std::unique_ptr<ComponentScript> ConstructComponent(EID id,
                                                      ComponentScript *parent);
  void SetDependencies(RenderManager *rm);

 private:
  lua_State *lState;
  LuaInterface *lInterface;
  RenderManager *dependencyRenderManager;
};

#endif
