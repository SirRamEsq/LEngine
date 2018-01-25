#ifndef L_COMPCOLLISIONBOX
#define L_COMPCOLLISIONBOX

#include "../BaseComponentManager.h"
#include "../LuaInclude.h"
#include "CompPosition.h"
#include "CompScript.h"

#include "../Defines.h"
#include "../Resources/RSC_Map.h"
#include "CollisionBox.h"
#include <list>
#include <functional>

/*
Primary collision box WILL send an event. It will also check if the other boxes
need to be checked
Thus, every entity that wants collision must have at least a primary cbox
*/

class ComponentCollisionManager;

class TColPacket {
 public:
  int tileX, tileY;   // tile coords
  CollisionBox *box;  // box id
  float xspd, yspd;   // speed of entity

  int GetTileX() { return tileX; }
  int GetTileY() { return tileY; }

  CollisionBox *GetBox() { return box; }
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

class EColPacket {
 public:
  CollisionBox *box;  // box id
  std::string name;
  std::string objType;

  CollisionBox *GetBox() { return box; }
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
  ComponentCollision(EID id, ComponentPosition *pos,
                     ComponentCollisionManager *manager);
  ~ComponentCollision();

  void DeactivateAll();
  void ActivateAll();

  void Update();

  /// Will create a collision box from a shape and return its id
  /// \param shape The shape that acts as the collidable area (can be NULL)
  CollisionBox *AddCollisionBox(const Shape *shape);
  /**
   * Sets the primary CollisionBox
   * If the primary box fails, the rest will not be checked for a given entity
   * \param boxid The id of the registered collision box to set as primary
   */
  void SetPrimaryCollisionBox(CollisionBox *box);

  void *extraData;

 protected:
  CollisionBox *GetPrimary();

  /// Registered CollisionBoxes
  std::unordered_map<int, CollisionBox> mBoxes;
  /// Active CollisionBoxes for entities
  std::vector<CollisionBox *> mActiveEntityBoxes;
  /// Active CollisionBoxes for tiles
  std::vector<CollisionBox *> mActiveTileBoxes;

 private:
  CollisionBox *GetColBox(int boxid);

  /// dependency
  ComponentPosition *myPos;

  int mNextBoxID;
};

// used to structure entity collisions
struct CollisionGrid {
  // Hash value to eid vector
  std::map<int, std::vector<EID>> buckets;

  void UpdateBuckets(const std::unordered_map<EID, ComponentCollision *> *comps,
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

  static void ExposeLuaInterface(lua_State *state);

 private:
  void SendCollisionEvent(const ComponentCollision &sender,
                          const ComponentCollision &reciever,
                          CollisionBox *recieverBox, Event::MSG mes);

  void RegisterTileCollision(TColPacket *packet, EID id,
                             CollisionBox::Callback callback = NULL);
};

#endif
