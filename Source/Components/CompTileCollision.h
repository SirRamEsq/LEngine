#ifndef L_COMPTILECOLLISION
#define L_COMPTILECOLLISION

#include "CompScript.h"

//Makes use of compCollisionBox

class ComponentTileCollision : public BaseComponent{
    public:
        ComponentTileCollision(EID id);

        void Update();
        void HandleEvent(Event* event);

    private:
        ComponentScript* myScript;
};

class ComponentTileCollisionManager : public BaseComponentManager{
    public:
        void AddComponent(EID id);
    private:

};

#endif
