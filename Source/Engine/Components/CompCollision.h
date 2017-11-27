#ifndef L_COMPCOLLISIONBOX
#define L_COMPCOLLISIONBOX

#include "../BaseComponentManager.h"
#include "CompPosition.h"
#include "CompScript.h"

#include "../Defines.h"
#include "../Resources/RSC_Map.h"
#include "CollisionBox.h"
#include <list>
#include <functional>

// Lua Includes
extern "C" {
#include "lua5.2/lauxlib.h"
#include "lua5.2/lua.h"
#include "lua5.2/lualib.h"
}
#include <LuaBridge.h>

/*
Primary collision box WILL send an event. It will also check if the other boxes
need to be checked
Thus, every entity that wants collision must have at least a primary cbox
*/

class ComponentCollisionManager;

struct TColPacket {
  int tileX, tileY;  // tile coords
  int posX, posY;    // actual position
  int box;           // box id
  float xspd, yspd;  // speed of entity

  int GetTileX() { return tileX; }
  int GetTileY() { return tileY; }

  int GetX() { return posX; }
  int GetY() { return posY; }

  int GetID() { return box; }
  RSC_Heightmap GetHmap();
  TiledTileLayer *GetLayer();
  TiledTileLayer *tl;

  /**
   * Handles setting a TColPacket and from event->extradata
   */
  class ExtraDataDefinition : public Event::ExtraDataDefinition {
   public:
    ExtraDataDefinition(const TColPacket *p);
    void SetExtraData(Event *event);
    static const TColPacket *GetExtraData(const Event *event);

   private:
    const TColPacket *packet;
  };
};

struct EColPacket {
  int box;  // box id
  std::string name;
  std::string objType;

  int GetID() { return box; }
  std::string GetName() { return name; }
  std::string GetType() { return objType; }

  /**
   * Handles setting an EColPacket and from event->extradata
   */
  class ExtraDataDefinition : public Event::ExtraDataDefinition {
   public:
    ExtraDataDefinition(const EColPacket *p);
    void SetExtraData(Event *event);
    static const EColPacket *GetExtraData(const Event *event);

   private:
    const EColPacket *packet;
  };
};

class ComponentCollision : public BaseComponent {
  friend class ComponentCollisionManager;
  friend struct CollisionGrid;

 public:
  // typedef void (*CollisionCallback)(const TColPacket *);
  typedef std::function<void(TColPacket *)> CollisionCallback;
  ComponentCollision(EID id, ComponentPosition *pos,
                     ComponentCollisionManager *manager);
  ~ComponentCollision();

  void Update();

  /// Add specified shape and id as a CollisionBox (should generate boxid
  /// instead of passing?)
  void AddCollisionBox(const Shape *shape, int boxid, int orderNum = 0);
  /**
   * Sets the primary CollisionBox
   * If the primary box fails, the rest will not be checked for a given entity
   * \param boxid The id of the registered collision box to set as primary
   */
  void SetPrimaryCollisionBox(int boxid);
  /**
   * Will enable a box ot be used to check for other entities
   * \param boxid	The id of the registered collision box to enable entity
   * checking on
   */
  void CheckForEntities(int boxid);
  /**
   * Will enable a box ot be used to check for tiles
   * \param boxid	The id of the registered collision box to enable tile
   * checking on
   */
  void CheckForTiles(int boxid);
  void Activate(int boxid);
  void Deactivate(int boxid);

  void SetName(std::string n) { name = n; }
  void SetType(std::string t) { objType = t; }

  void SetShape(int boxid, const Shape *shape);

  void CheckForLayer(int boxid, TiledTileLayer *layer,
                     CollisionCallback callback);

  void CheckForLayerLuaInterface(int boxid, TiledLayerGeneric *layer,
                                 luabridge::LuaRef func);

  std::string name;
  std::string objType;
  void *extraData;

 protected:
  CollisionBox *GetPrimary();

  auto GetItBeg() { return boxes.begin(); }
  auto GetItEnd() { return boxes.end(); }

  typedef std::unordered_map<TiledTileLayer *, CollisionCallback>
      LayerCallbacks;
  std::unordered_map<int, LayerCallbacks> mLayersToCheck;

 private:
  void OrderList();
  CollisionBox *GetColBox(int boxid);

  /// Registered CollisionBoxes
  std::unordered_map<int, CollisionBox> boxes;

  /// dependency
  ComponentPosition *myPos;
};

// used to structure entity collisions
struct CollisionGrid {
  // Hash value to eid vector
  std::map<int, std::vector<EID>> buckets;

  void UpdateBuckets(
      const std::unordered_map<EID, std::unique_ptr<ComponentCollision>> *comps,
      int mapWidthPixels);
};
class ComponentCollisionManager
    : public BaseComponentManager_Impl<ComponentCollision> {
 public:
  ComponentCollisionManager(EventDispatcher *e);
  void Update();
  std::unique_ptr<ComponentCollision> ConstructComponent(
      EID id, ComponentCollision *parent);
  void UpdateBuckets(int widthPixels);
  void UpdateCheckEntityCollision();
  void UpdateCheckTileCollision(RSC_Map *currentMap);
  void SetDependencies(ComponentPositionManager *pos);

  CollisionGrid grid;
  ComponentPositionManager *dependencyPosition;

  /**
   * Will return the first layer collided with in the vector
   * \param x Tile Coordinate X
   * \param y Tile Coordinate Y
   */
  static TiledTileLayer *GetTileLayerCollision(
      const std::vector<TiledTileLayer *> *layers, unsigned int x,
      unsigned int y);

 private:
  void SendCollisionEvent(const ComponentCollision &sender,
                          const ComponentCollision &reciever, int recieverBoxID,
                          Event::MSG mes);

  void RegisterTileCollision(
      TColPacket *packet, EID id,
      ComponentCollision::CollisionCallback callback = NULL);
};

#endif
