#ifndef L_ENTITY_MAN
#define L_ENTITY_MAN

#include "BaseComponentManager.h"
#include "Defines.h"
#include "Event.h"
#include "Exceptions.h"

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "LuaInclude.h"

class GameStateManager;

/**
 * Manages Entity IDs (EIDs), Component Managers, and ensures that deleted
 * entities are fully deleted at the appropriate time
 */
class EntityManager {
 public:
  class Exception : public LEngineException {
    using LEngineException::LEngineException;
  };
  /**
   * Defines an order to update the component managers in an order that should
   * be acceptable to most GameStates
   */
  enum DEFAULT_UPDATE_ORDER {
    INPUT = 10,
    PARTICLE = 20,
    POSITION = 30,
    COLLISION = 40,
    SCRIPT = 50,
    CAMERA = 60,
    LIGHT = 70,
    SPRITE = 80
  };

  EntityManager(GameStateManager *manager);

  /**
    * Creates a New Entity
    * \return The new Entity ID value
    */
  EID NewEntity();

  /**
    * Creates a New Entity
  * Maps name to returned eid
    * \return The new Entity ID value
    */
  EID NewEntity(const std::string &entityName);

  /**
   * Will map the passed name to the passed EID
   */
  void SetName(EID id, const std::string &name);
  /**
   * Can lookup an eid by a string name, if the entity was given one upon
   * creation
   */
  EID GetEIDFromName(const std::string &name) const;

  /**
   * Schedules all components with the given entity to be deleted from their
   * managers
   */
  void DeleteEntity(EID id);
  /**
   * Calls 'DeleteEntity' on all active eids
   * This deletes only the currently existing entities. If you call this
   * function
   * then call 'NewEntity' the entity created will not be deleted next frame
   * \see DeleteEntity(EID id)
   */
  void ClearAllEntities();

  /**
   * Deletes all EIDs between first reserved EID and EID_MIN
   * \see DeleteEntity(EID id)
   */
  void ClearAllReservedEntities();

  /**
   * Dispatches an Event to all registered Component Managers
   */
  void DispatchEvent(const Event *event);

  /**
   * Broadcasts an event to all registered Component Managers
   */
  void BroadcastEvent(const Event *event);

  /**
   * Actually deletes all entities scheduled to be deleted.
   */
  void Cleanup();

  /**
   * Can Register a new Component Manager to recieve events, have entities added
   * to/deleted from, etc...
   */
  void RegisterComponentManager(BaseComponentManager *manager, int order);

  /**
   * Returns a count of currently active entities
   */
  unsigned int GetEntityCount();

  /**
   * Sets the parent of each component of an entity
   */
  void SetParent(EID child, EID parent);

  /// Deactivates all Entities
  void DeactivateAll();
  /// Deactivates all Entities except the ones specified
  void DeactivateAllExcept(const std::vector<EID> &entities);
  /// Activates all Entities
  void ActivateAll();
  /// Activates all Entities except the ones specified
  void ActivateAllExcept(const std::vector<EID> &entities);
  /// Activate specified entities
  void Activate(const std::vector<EID> &entities);
  /// Deactivate specified entities
  void Deactivate(const std::vector<EID> &entities);
  /// Activate specified entities
  void Activate(EID entities);
  /// Deactivate specified entities
  void Deactivate(EID entities);

  static void ExposeLuaInterface(lua_State *state);

 protected:
  /// Clears out nameToEID and EIDToName
  void ClearNameMappings();

 private:
  void MapNameToEID(EID eid, const std::string &entityName);

  /// Current highest EID in use
  EID maxInUseEID;

  /// Set of entities to be deleted next frame
  std::unordered_set<EID> deadEntities;

  /// Set of in use EIDS
  std::unordered_set<EID> aliveEntities;

  /// entities that are active
  std::unordered_set<EID> activeEntities;
  /// entities that are inactive
  std::unordered_set<EID> inactiveEntities;

  /// container of EIDS that have been deleted and can be reused
  std::vector<EID> reclaimedEIDs;

  /// Map of names to EIDS and an inverse lookup map (this is fine because both
  /// the names and EIDs are unique
  std::map<std::string, EID> nameToEID;
  std::map<EID, std::string> EIDToName;

  /// Components sorted by priority in ascending order
  std::map<int, BaseComponentManager *, std::less<int>> componentsRegistered;

  /// Dependency
  GameStateManager *mStateManager;
};

#endif
