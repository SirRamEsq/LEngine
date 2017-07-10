#ifndef L_COMPINPUT
#define L_COMPINPUT

#include "CompScript.h"
#include "../Input.h"
#include <string>

class ComponentInput : public BaseComponent{
    public:
        ComponentInput(InputManager::KeyMapping* keys, EID id, ComponentScript* script, const std::string& logFile);

        void Update();
        void HandleEvent(const Event* event);
        void ListenForInput(std::string keyName);

    private:
        ComponentScript* myScript;

        InputManager::KeyMapping* keyMapping;
};

class ComponentInputManager : public BaseComponentManager{
    public:
        ComponentInputManager(EventDispatcher* e);
        void SetDependency(std::shared_ptr<InputManager::KeyMapping> keys);

        void AddComponent(EID id);
    private:
        std::shared_ptr<InputManager::KeyMapping> keyMapping;
};

#endif
