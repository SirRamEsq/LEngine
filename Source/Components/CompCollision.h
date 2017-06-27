#ifndef L_COMPCOLLISIONBOX
#define L_COMPCOLLISIONBOX

#include "CompPosition.h"
#include "CompScript.h"

#include "../Resources/RSC_Map.h"
#include "../Defines.h"
#include <list>

//Lua Includes
extern "C"{
  #include "lua5.2/lua.h"
  #include "lua5.2/lualib.h"
  #include "lua5.2/lauxlib.h"
}
#include <LuaBridge.h>


/*
Primary collision box WILL send an event. It will also check if the other boxes need to be checked
Thus, every entity that wants collision must have at least a primary cbox
*/

enum bitflags{
    //Always check is now ignored with the advent of buckets
    ALWAYS_CHECK=1,
    PRIMARY=2,
    ENT_CHECK=4,
    TILE_CHECK=8
};

class ComponentCollisionManager;

struct TColPacket{
    int x, y;//tile coords, not screen position
    int box; //box id
    int GetX(){return x;}
    int GetY(){return y;}
    int GetID(){return box;}
    LHeightmap GetHmap();
    /*LTDF* GetTDF(){return tdf;}
    LTDF* tdf;//tdf collided with*/
    const TiledTileLayer* GetLayer();
    const TiledTileLayer* tl;
};

struct EColPacket{
    int box; //box id
    std::string name;
    std::string objType;

    int GetID(){return box;}
    std::string GetName(){return name;}
    std::string GetType(){return objType;}
};


class ComponentCollision : public BaseComponent{
    friend class ComponentCollisionManager;
    friend class CollisionGrid;

    public:
        struct ColBox{
            //implemented for vector sorting
            bool operator < (const ColBox &rhs) const;

            ColBox(CRect r, int i, ComponentPosition* pos, uint8_t f, int orderNum, MAP_DEPTH depth=0);
            CRect rect;
            CRect gameCoords;
            int id;
            uint8_t flags;
            MAP_DEPTH depth;
            ComponentPosition* myPos;
            const CRect& ToGameCoords();
            int mOrderNum; //Higher order nums get placed first

            bool active;
        };
        ComponentCollision(EID id, ComponentPosition* pos, const std::string& logFile);
        ~ComponentCollision();
        bool noTiles;//Don't check against any tiles ever

        void Update();

        void AddCollisionBoxInt(int x, int y, int w, int h, int boxid, int orderNum=0); //x and y are relative to myPos
        void AddCollisionBox(CRect rect, int boxid, int orderNum=0);
        void SetPrimaryCollisionBox(int boxid, bool ntile=false);//If collision fails with the primary box, none of the others are checked
        void AlwaysCheck(int boxid); //this box will always be checked, even if the primary fails
        void CheckForEntities(int boxid);
        void CheckForTiles(int boxid);
        void Activate(int boxid);
        void Deactivate(int boxid);

        void ChangeWidth(int boxid, int value);
        void ChangeHeight(int boxid, int value);
        void ChangeX(int boxid, int value);
        void ChangeY(int boxid, int value);

        void SetName(std::string n){name=n;}
        void SetType(std::string t){objType=t;}

        void ChangeBox(int boxid, CRect& box);

        std::string name;
        std::string objType;
        void*       extraData;

    protected:
        ColBox* GetPrimary();
        CRect* GetPrimaryRect();
        CRect* GetBox(int boxid);

        std::vector<ColBox>::iterator GetItBeg(){return boxes.begin();}
        std::vector<ColBox>::iterator GetItEnd(){return boxes.end();}

    private:
        void OrderList();
        int alwaysCheckCount; //number of boxes to always check
        ColBox* GetColBox(int boxid);
        std::vector<ColBox> boxes;
        ComponentPosition* myPos;
};

//used to structure entity collisions
struct CollisionGrid{
    //Hash value to eid vector
    std::map< int, std::vector<EID> > buckets;

    void UpdateBuckets(std::map<EID, ComponentCollision*>* comps, int mapWidthPixels);
};
class ComponentCollisionManager : public BaseComponentManager{
    public:
        ComponentCollisionManager(EventDispatcher* e);
        void AddComponent(EID id);
        void Update();
        void UpdateBuckets(int widthPixels);
        void UpdateCheckEntityCollision ();
        void UpdateCheckTileCollision   (const I_RSC_Map* currentMap);

        CollisionGrid grid;

        ComponentPositionManager* dependencyPosition;

    private:
        void SendCollisionEvent(const ComponentCollision& sender, const ComponentCollision& reciever, int recieverBoxID, MESSAGE_TYPE mes);
};

#endif

