#ifndef L_POSCOMP
#define L_POSCOMP

#include "../BaseComponent.h"
#include <set>
class ComponentPosition;
class MapNode{
    friend ComponentPosition;
    typedef std::set<MapNode*> tNodeSet;

    public:
        MapNode (MapNode* parent);
        ~MapNode();

        void UpdateWorld(const Coord2df& worldCoordinates, bool dirty);
        // The coordinates passed are transformed into either world or local
        void TranslateLocalToWorld(Coord2df& worldCoordinates);
        void TranslateWorldToLocal(Coord2df& worldCoordinates);

        void ChangeParent(MapNode* parent);
        const Coord2df * GetWorldPositionReference(){return &mPositionWorld;}

        MapNode* mParent;
        bool mDirty;

        Coord2df mPositionLocal;
        Coord2df mPositionWorld;

        MapNode* GetRootNode();

    protected:
        bool RemoveChild(MapNode* child);
        void AddChild   (MapNode* child);

        tNodeSet mChildren;
};

class ComponentPosition : public BaseComponent{
    public:
        ComponentPosition(EID id, MapNode* parent, const std::string& logFile);
        ~ComponentPosition();

        void Update();
        void HandleEvent(const Event* event);

        Coord2df GetPositionLocal() {return mNode->mPositionLocal;}
        Coord2df GetPositionWorld() {return mNode->mPositionWorld;}
        Coord2df GetMovement()      {return mMovement;}
        Coord2df GetAcceleration()  {return mAcceleration;}
        Coord2d  GetPositionWorldInt();
        Coord2d  GetPositionLocalInt();
        const Coord2df * GetWorldPositionReference(){return mNode->GetWorldPositionReference();}


        void SetPositionLocal   (Coord2df pos);
        void SetPositionLocalInt(Coord2d  pos);
        void SetMovement        (Coord2df mov);
        void SetAcceleration    (Coord2df acl);
        void SetMaxSpeed        (float speed ){maximumSpeed=speed;}

        void SetPositionLocalX  (float x);
        void SetMovementX       (float x);
        void SetAccelerationX   (float x);

        void SetPositionLocalY  (float y);
        void SetMovementY       (float y);
        void SetAccelerationY   (float y);

        void IncrementPosition      (Coord2df pos);
        void IncrementMovement      (Coord2df mov);
        void IncrementAcceleration  (Coord2df accel);


        void RoundPosition();
        void ChangeParent(EID id);
        Coord2df TranslateWorldToLocal(Coord2df world);
        Coord2df TranslateLocalToWorld(Coord2df local);

        MapNode* GetMapNode(){return mNode;}

    private:
        Coord2df mMovement;
        Coord2df mAcceleration;
        float maximumSpeed;

        MapNode* mNode;
};

class ComponentPositionManager : public BaseComponentManager{
    public:
        ComponentPositionManager();
        ~ComponentPositionManager();

        void Update();
        void AddComponent(EID id);
        void AddComponent(EID id, MapNode* parent);
        void HandleEvent(const Event* event){}

        MapNode* const GetRootNode(){return mRootNode;}

        static const int collisionGridDimension;

    private:
        MapNode* mRootNode;
};

#endif
