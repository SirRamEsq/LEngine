#ifndef L_COMPINPUT
#define L_COMPINPUT

#include "CompScript.h"
#include "../Input.h"
#include <string>

class ComponentInput : public BaseComponent{
    public:
        ComponentInput(EID id, ComponentScript* script, const std::string& logFile);

        void Update();
        void HandleEvent(const Event* event);
        void ListenForInput(std::string keyName);

    private:
        ComponentScript* myScript;
};

class ComponentInputManager : public BaseComponentManager{
    public:
        ComponentInputManager();

        void AddComponent(EID id);
    private:

};

#endif
