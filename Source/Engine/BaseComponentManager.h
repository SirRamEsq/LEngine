#ifndef LENGINE_BASE_COMP_MAN
#define LENGINE_BASE_COMP_MAN

#include "BaseComponent.h"
#include "Defines.h"
#include <unordered_map>

class BaseComponentManager {
 public:
  /// Create a new component with the specified ID
  virtual void AddComponent(EID id, EID parent = 0) = 0;

  virtual void DeleteAllComponents() = 0;
  virtual void DeleteComponent(EID id) = 0;

  virtual bool HasComponent(EID id) = 0;
  virtual bool IsActive(EID id) = 0;
  virtual int GetComponentCount() = 0;
  virtual int GetActiveComponentCount() = 0;

  virtual void ActivateComponent(EID id) = 0;
  virtual void DeactivateComponent(EID id) = 0;
  /**
   * Updates all active components once
   * Sets and unsets the component's 'updatedThisFrame' boolean
   **/
  virtual void Update() = 0;
  /**
   * Handles recieved events
   * If an entity is deleted that acts as a parent for another entity
   * the componentManager should set that entity's parent to NULL
   * The component that has the deleted entity as a parent may also access the
   * old parent
   * and change parents before the manager forces the parent to be null
   */
  virtual void HandleEvent(const Event *event) = 0;
  virtual void BroadcastEvent(const Event *event) = 0;
  /**
   * Sets parent of a component
   * Both parent and child must exist as a component of this manager
   * if parent is 0, then parent of the child is set to NULL
   */
  virtual void SetParent(EID child, EID parent) = 0;
};

template <class T>
class BaseComponentManager_Impl : public BaseComponentManager {
  // Template should have BaseComponent as Base
  static_assert(std::is_base_of<BaseComponent, T>::value,
                "T must derive from BaseComponent");

 public:
  BaseComponentManager_Impl(EventDispatcher *e);

  /// Create a new component with the specified ID
  void AddComponent(EID id, EID parent = 0) final;
  /// Add an instantiated component
  void AddComponent(std::unique_ptr<T> comp);

  /// How Component 'T' is actually instantiated
  virtual std::unique_ptr<T> ConstructComponent(EID id, T *parent) = 0;

  void DeleteAllComponents() final;
  void DeleteComponent(EID id) final;

  bool HasComponent(EID id) final;
  virtual T *GetComponent(EID id);
  virtual const T *GetConstComponent(EID id) const;
  bool IsActive(EID id) final;
  int GetComponentCount() final;
  int GetActiveComponentCount() final;

  void ActivateComponent(EID id) final;
  void DeactivateComponent(EID id) final;

  /**
   * Updates all active components once
   * Sets and unsets the component's 'updatedThisFrame' boolean
   **/
  virtual void Update();
  /**
   * Handles recieved events
   * If an entity is deleted that acts as a parent for another entity
   * the componentManager should set that entity's parent to NULL
   * The component that has the deleted entity as a parent may also access the
   * old parent
   * and change parents before the manager forces the parent to be null
   */
  void HandleEvent(const Event *event) final;
  void BroadcastEvent(const Event *event) final;
  /**
   * Sets parent of a component
   * Both parent and child must exist as a component of this manager
   * if parent is 0, then parent of the child is set to NULL
   */
  void SetParent(EID child, EID parent);

 protected:
  /**
   * Recursive function, updates parents before children
   * sets 'updatedThisFrame' on components after being updated
   * otherwise the entities are ordered and updated by EID
   */
  void UpdateComponent(T *child);
  virtual void ProcessEvent(const Event *event);

  /// All Components that this Manager owns
  std::unordered_map<EID, std::unique_ptr<T>> componentList;
  /// Components that are active and will be updated
  std::unordered_map<EID, T *> activeComponents;
  EventDispatcher *eventDispatcher;
};

// Include definitions
#include "BaseComponentManager.tpp"

#endif
