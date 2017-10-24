#ifndef LENGINE_BASE_COMP
#define LENGINE_BASE_COMP

#include "Defines.h"
#include "Event.h"

#include <memory>

class ComponentPositionManager;
class BaseComponentManager;
class BaseComponent {
  friend class BaseComponentManager;
  friend class ComponentPositionManager;

  // callback function called on event
  typedef void (*EventFunction)(const Event *event);

 public:
  virtual ~BaseComponent();

  BaseComponent(EID id, BaseComponentManager *manager);

  virtual void Update() = 0;
  virtual void HandleEvent(const Event *event);

  EID GetEID() const { return mEntityID; }

  void SetEventCallbackFunction(EventFunction f);

  virtual void SetParent(BaseComponent *p);
  /**
   * Calls mManager's SetParent with own id and argument id
   * Mainly exists for the sake of the scripting language
   */
  void SetParentEID(EID id);
  virtual BaseComponent *GetParent();

  // Used by ComponentManager to determine if this component was already updated
  // this frame
  bool updatedThisFrame;

 protected:
  EID mEntityID;

  /// Pointer to component's parent, should be same type of component as derived
  /// class BaseComponent* parent;
  BaseComponent *parent;

 private:
  /// Can be used to handle an event (optional)
  EventFunction eventCallback;

  BaseComponentManager *mManager;
};

#endif
