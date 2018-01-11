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
   * Also adds new components to the component list
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
  void AddNewComponents();
 protected:
  /**
   * Recursive function, updates parents before children
   * sets 'updatedThisFrame' on components after being updated
   * otherwise the entities are ordered and updated by EID
   */
  void UpdateComponent(T *child);
  virtual void ProcessEvent(const Event *event);

  /// All Components that this Manager owns
  std::unordered_map<EID, std::unique_ptr<T>> mComponentList;
  /// Components to add at the start of the next frame
  std::unordered_map<EID, std::unique_ptr<T>> mNewComponents;
  /// Components that are active and will be updated
  std::unordered_map<EID, T *> mActiveComponents;
  EventDispatcher *mEventDispatcher;
};

template <class T>
BaseComponentManager_Impl<T>::BaseComponentManager_Impl(EventDispatcher *e)
    : mEventDispatcher(e) {}

template <class T>
int BaseComponentManager_Impl<T>::GetComponentCount() {
  auto size1 = mComponentList.size();
  auto size2 = mNewComponents.size();
  return size1 + size2;
}

template <class T>
int BaseComponentManager_Impl<T>::GetActiveComponentCount() {
  return mActiveComponents.size();
}

template <class T>
void BaseComponentManager_Impl<T>::DeleteAllComponents() {
  mComponentList.clear();
  mNewComponents.clear();
  mActiveComponents.clear();
}

template <class T>
void BaseComponentManager_Impl<T>::DeleteComponent(EID id) {
  mComponentList.erase(id);
  mNewComponents.erase(id);
  mActiveComponents.erase(id);
}

template <class T>
bool BaseComponentManager_Impl<T>::HasComponent(EID id) {
  if (mComponentList.find(id) != mComponentList.end()) {
    return true;
  }
  if (mNewComponents.find(id) != mNewComponents.end()) {
    return true;
  }
  return false;
}

template <class T>
bool BaseComponentManager_Impl<T>::IsActive(EID id) {
  return (mActiveComponents.find(id) != mActiveComponents.end());
}

template <class T>
void BaseComponentManager_Impl<T>::UpdateComponent(T *child) {
  // Exit if already updated this frame
  if (child->updatedThisFrame == true) {
    return;
  }

  // Update parent before child
  auto parent = child->GetParent();
  if (parent != NULL) {
    auto parentEID = parent->GetEID();
    auto parentIterator = mActiveComponents.find(parentEID);
    if (parentIterator != mActiveComponents.end()) {
      // Recurse update function with parent
      UpdateComponent(parentIterator->second);
    }
  }

  // Update child
  child->Update();
  child->updatedThisFrame = true;
}

template <class T>
void BaseComponentManager_Impl<T>::Update() {
  AddNewComponents();

  // Update all entities
  for (auto i = mActiveComponents.begin(); i != mActiveComponents.end(); i++) {
    UpdateComponent(i->second);
  }

  // Reset all 'updatedThisFrame' bits
  for (auto i = mActiveComponents.begin(); i != mActiveComponents.end(); i++) {
    i->second->updatedThisFrame = false;
  }
}

template <class T>
void BaseComponentManager_Impl<T>::HandleEvent(const Event *event) {
  EID recieverEID = event->reciever;
  BaseComponent *comp = GetComponent(recieverEID);
  if (comp == NULL) {
    return;
  }

  ProcessEvent(event);

  comp->HandleEvent(event);
}
template <class T>
void BaseComponentManager_Impl<T>::ProcessEvent(const Event *event) {
  EID recieverEID = event->reciever;
  if (recieverEID == EID_ALLOBJS) {
    return;
  }
  BaseComponent *comp = GetComponent(recieverEID);
  if (comp == NULL) {
    return;
  }

  if (event->message == Event::MSG::ENTITY_DELETED) {
    // if component has a parent, it needs to be changed if the parent was
    // deleted
    auto parent = comp->GetParent();
    if (parent != NULL) {
      if (parent->GetEID() == event->sender) {
        // Set parent to NULL
        SetParent(recieverEID, 0);
      }
    }
  }
}

template <class T>
void BaseComponentManager_Impl<T>::BroadcastEvent(const Event *event) {
  ProcessEvent(event);
  for (auto i = mActiveComponents.begin(); i != mActiveComponents.end(); i++) {
    if (i->second->GetEID() != event->sender) {
      i->second->HandleEvent(event);
    }
  }
}

template <class T>
T *BaseComponentManager_Impl<T>::GetComponent(EID id) {
  auto i = mComponentList.find(id);
  if (i != mComponentList.end()) {
    return i->second.get();
  }

  i = mNewComponents.find(id);
  if (i != mNewComponents.end()) {
    return i->second.get();
  }

  return NULL;
}

template <class T>
const T *BaseComponentManager_Impl<T>::GetConstComponent(EID id) const {
  auto i = mComponentList.find(id);
  if (i != mComponentList.end()) {
    return i->second.get();
  }

  i = mNewComponents.find(id);
  if (i != mNewComponents.end()) {
    return i->second.get();
  }

  return NULL;
}

template <class T>
void BaseComponentManager_Impl<T>::AddComponent(std::unique_ptr<T> comp) {
  if (comp.get() == NULL) {
    return;
  }
  auto id = comp->GetEID();

  // This shouldn't happen
  ASSERT(mComponentList.find(id) == mComponentList.end());
  if (mComponentList.find(id) != mComponentList.end()) {
    return;
  }

  mNewComponents[id] = std::move(comp);
}

template <class T>
void BaseComponentManager_Impl<T>::AddNewComponents() {
  for (auto i = mNewComponents.begin(); i != mNewComponents.end(); i++) {
    mComponentList[i->first] = std::move(i->second);
    ActivateComponent(i->first);
  }
  mNewComponents.clear();
}

template <class T>
void BaseComponentManager_Impl<T>::AddComponent(EID id, EID parent) {
  T *parentPointer = NULL;

  auto i = mComponentList.find(id);
  if (i != mComponentList.end()) {
    parentPointer = i->second.get();
  }

  AddComponent(ConstructComponent(id, parentPointer));
}

template <class T>
void BaseComponentManager_Impl<T>::SetParent(EID child, EID parent) {
  if (child == 0) {
    return;
  }

  auto childComponent = GetComponent(child);
  if (childComponent == NULL) {
    return;
  }
  if (parent == 0) {
    childComponent->SetParent(NULL);
    return;
  }

  auto parentComponent = GetComponent(parent);
  if (parentComponent == NULL) {
    return;
  }

  childComponent->SetParent(parentComponent);
}

template <class T>
void BaseComponentManager_Impl<T>::ActivateComponent(EID id) {
  auto comp = mComponentList.find(id);
  if (comp != mComponentList.end()) {
    mActiveComponents[id] = comp->second.get();
  }
}

template <class T>
void BaseComponentManager_Impl<T>::DeactivateComponent(EID id) {
  auto comp = mActiveComponents.find(id);
  if (comp != mActiveComponents.end()) {
    mActiveComponents.erase(comp);
  }
}

#endif
