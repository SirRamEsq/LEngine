#ifndef L_COMPENTITYCOLLISION
#define L_COMPENTITYCOLLISION

#include "CompScript.h"
#include "CompPosition.h"

//Makes use of compCollisionBox

class ComponentEntityCollision : public BaseComponent{
    public:
        ComponentEntityCollision(EID id, const std::string& logFile);

        void Update();
        void HandleEvent(Event* event);

    private:
        ComponentScript* myScript;
        ComponentPosition* myPos;
};

class ComponentEntityCollisionManager : public BaseComponentManager{
    public:
        void AddComponent(EID id);
    private:

};

#endif
