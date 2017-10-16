#ifndef L_COMPCOLLISIONBOX
#define L_COMPCOLLISIONBOX

#include "../BaseComponentManager.h"
#include "CompPosition.h"
#include "CompScript.h"

#include "../Resources/RSC_Map.h"
#include "../Defines.h"
#include "CollisionBox.h"
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

class ComponentCollisionManager;

struct TColPacket{
    int x, y;//tile coords, not screen position
    int box; //box id
    int GetX(){return x;}
    int GetY(){return y;}
    int GetID(){return box;}
    RSC_Heightmap GetHmap();
    /*LTDF* GetTDF(){return tdf;}
    LTDF* tdf;//tdf collided with*/
    const TiledTileLayer* GetLayer();
    const TiledTileLayer* tl;
	
		/**
		 * Handles setting a TColPacket and from event->extradata
		 */
		class ExtraDataDefinition : public Event::ExtraDataDefinition{
			public:
				ExtraDataDefinition(const TColPacket* p);
				void SetExtraData(Event* event);
				static const TColPacket* GetExtraData(const Event* event);	

			private:
				const TColPacket* packet;
		};
};

struct EColPacket{
    int box; //box id
    std::string name;
    std::string objType;

    int GetID(){return box;}
    std::string GetName(){return name;}
    std::string GetType(){return objType;}

	
		/**
		 * Handles setting an EColPacket and from event->extradata
		 */
		class ExtraDataDefinition : public Event::ExtraDataDefinition{
			public:
				ExtraDataDefinition(const EColPacket* p);
				void SetExtraData(Event* event);
				static const EColPacket* GetExtraData(const Event* event);	

			private:
				const EColPacket* packet;
		};
};


class ComponentCollision : public BaseComponent{
    friend class ComponentCollisionManager;
    friend class CollisionGrid;

    public:
        ComponentCollision(EID id, ComponentPosition* pos, ComponentCollisionManager* manager);
        ~ComponentCollision();

        void Update();

        void AddCollisionBoxInt(int x, int y, int w, int h, int boxid, int orderNum=0); //x and y are relative to myPos
        void AddCollisionBox(Rect rect, int boxid, int orderNum=0);
        void SetPrimaryCollisionBox(int boxid);//If collision fails with the primary box, none of the others are checked
        void CheckForEntities(int boxid);
        void CheckForTiles(int boxid);
        void Activate(int boxid);
        void Deactivate(int boxid);

        void SetName(std::string n){name=n;}
        void SetType(std::string t){objType=t;}

        void SetShape(int boxid, const Shape* shape);

        std::string name;
        std::string objType;
        void*       extraData;

    protected:
        CollisionBox* GetPrimary();

        std::vector<CollisionBox>::iterator GetItBeg(){return boxes.begin();}
        std::vector<CollisionBox>::iterator GetItEnd(){return boxes.end();}

    private:
        void OrderList();
        int alwaysCheckCount; //number of boxes to always check
        CollisionBox* GetColBox(int boxid);
        std::vector<CollisionBox> boxes;
        ComponentPosition* myPos;
};

//used to structure entity collisions
struct CollisionGrid{
    //Hash value to eid vector
    std::map< int, std::vector<EID> > buckets;

    void UpdateBuckets(const std::unordered_map<EID, std::unique_ptr<ComponentCollision> >* comps, int mapWidthPixels);
};
class ComponentCollisionManager : public BaseComponentManager_Impl<ComponentCollision>{
    public:
        ComponentCollisionManager(EventDispatcher* e);
		void Update();
		std::unique_ptr<ComponentCollision>ConstructComponent(EID id, ComponentCollision* parent);
        void UpdateBuckets(int widthPixels);
        void UpdateCheckEntityCollision ();
        void UpdateCheckTileCollision   (const RSC_Map* currentMap);
		void SetDependencies(ComponentPositionManager* pos);

        CollisionGrid grid;
        ComponentPositionManager* dependencyPosition;

    private:
        void SendCollisionEvent(const ComponentCollision& sender, const ComponentCollision& reciever, int recieverBoxID, Event::MSG mes);
};

#endif

