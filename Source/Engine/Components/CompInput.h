#ifndef L_COMPINPUT
#define L_COMPINPUT

#include "../BaseComponentManager.h"
#include "../Input.h"
#include "CompScript.h"
#include <string>

/**
 * This component is used to register the keys to listen to for a given entity
 * it does not decide what to do with the input, it is only responsible for
 * getting the input
 */
class ComponentInputManager;
class ComponentInput : public BaseComponent {
 public:
  ComponentInput(InputManager::KeyMapping *keys, EID id,
                 ComponentInputManager *manager);
  ~ComponentInput();

  void Update();
  void HandleEvent(const Event *event);
  void ListenForInput(std::string keyName);

 private:
  InputManager::KeyMapping *keyMapping;
};

class ComponentInputManager : public BaseComponentManager_Impl<ComponentInput> {
 public:
  ComponentInputManager(EventDispatcher *e);
  void SetDependency(std::shared_ptr<InputManager::KeyMapping> keys);
  std::unique_ptr<ComponentInput> ConstructComponent(EID id,
                                                     ComponentInput *parent);

 private:
  std::shared_ptr<InputManager::KeyMapping> keyMapping;
};

#endif
