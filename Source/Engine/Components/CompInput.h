#ifndef L_COMPINPUT
#define L_COMPINPUT

#include "CompScript.h"
#include "../Input.h"
#include <string>

/**
 * This component is used to register the keys to listen to for a given entity
 * it does not decide what to do with the input, it is only responsible for getting the input
 */
class ComponentInput : public BaseComponent{
    public:
        ComponentInput(InputManager::KeyMapping* keys, EID id, const std::string& logFile);
		~ComponentInput();

        void Update();
        void HandleEvent(const Event* event);
        void ListenForInput(std::string keyName);

    private:
        InputManager::KeyMapping* keyMapping;
};

class ComponentInputManager : public BaseComponentManager{
    public:
        ComponentInputManager(EventDispatcher* e);
        void SetDependency(std::shared_ptr<InputManager::KeyMapping> keys);
        void AddComponent(EID id, EID parent=0);

    private:
        std::shared_ptr<InputManager::KeyMapping> keyMapping;
};

#endif
