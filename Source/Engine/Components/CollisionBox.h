#ifndef LENGINE_COMP_COLLISION_BOX
#define LENGINE_COMP_COLLISION_BOX

#include "../Coordinates.h"
#include "../Defines.h"

#include "../Errorlog.h"
#include <unordered_map>
#include <functional>
#include <iostream>

#include "../LuaInclude.h"

enum CB_FLAGS { PRIMARY = 1, ENT_CHECK = 2, TILE_CHECK = 4 };

class ComponentPosition;
class ComponentCollision;
class ComponentCollisionManager;
class TiledTileLayer;
class TiledLayerGeneric;
class TColPacket;

class CollisionBox {
  friend ComponentCollision;
  friend ComponentCollisionManager;

 public:
  typedef std::vector<CollisionBox *> CB_Vector;
  typedef std::function<void(TColPacket *)> Callback;

  CollisionBox(int id, int order, const Shape *shape, ComponentPosition *pos,
               CB_Vector *activeTile, CB_Vector *activeEntity);
  bool operator<(const CollisionBox &rhs) const;
  CollisionResponse Collides(const CollisionBox *box);

  void Activate();
  void Deactivate();
  bool IsActive() const;

  bool IsPrimary() const;

  void SetShape(const Shape *shape);
  void UpdateWorldCoord();
  const Shape *GetWorldCoord();

  void SetOrder(int order);

  void RegisterFirstTileCollision();
  void RegisterEveryTileCollision();

  void CheckForLayer(TiledTileLayer *layer, Callback callback);
  void CheckForLayer(std::vector<TiledTileLayer *> *layers, Callback callback);
  void CheckForLayerLuaInterface(TiledLayerGeneric *layer,
                                 luabridge::LuaRef func);
  void CheckForLayersLuaInterface(std::vector<TiledLayerGeneric *> *layers,
                                  luabridge::LuaRef func);

  /**
   * Will enable a box ot be used to check for other entities
   * \param boxid	The id of the registered collision box to enable entity
   * checking on
   */
  void CheckForEntities();
  /**
   * Will enable a box ot be used to check for tiles
   * \param boxid	The id of the registered collision box to enable tile
   * checking on
   */
  void CheckForTiles();

  int GetID() { return mBoxID; }
  CB_Vector *mActiveTileBoxes;
  CB_Vector *mActiveEntityBoxes;

 protected:
  typedef std::pair<TiledTileLayer *, Callback> LayerCallbacks;
  std::vector<LayerCallbacks> mLayersToCheck;
  bool mPrimary;

 private:
  void AddSelfToVector(CB_Vector *vec);
  void RemoveSelfFromVector(CB_Vector *vec);
  /// Identifies whether this box is currently active
  bool mActive;

  /// Identifies the order in which this box is processed
  /// Higher orders are processed first
  int mOrder;

  /// Identifies this box
  int mBoxID;

  bool mCheckTiles;
  bool mCheckEntities;

  /// Whether or not box should only register the first tile collision per frame
  /// or all collisions perframe
  bool mReturnOnlyFirstTileCollision;

  /// Dependency on compPosition... may remove
  ComponentPosition *mPos;

  std::unique_ptr<Shape> mShape;
  std::unique_ptr<Shape> mShapeToWorldCoords;
};
#endif
