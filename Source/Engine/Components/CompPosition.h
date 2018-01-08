#ifndef L_POSCOMP
#define L_POSCOMP

#include "../BaseComponentManager.h"
#include "../Coordinates.h"
#include <set>

// Forward-Declare for MapNode
class ComponentPosition;

/**
 * This struct represents the current position of an entity on the map relative
 * to its parent
 */
struct MapNode {
  friend ComponentPosition;

  // Functions
  MapNode();
  ~MapNode();

  /**
   * Update World Position from local coordinates and parent's world coordinates
   */
  void UpdateWorld();

  /**
   * Transforms Coordinates relative to this node into World Coordinates
   * This function should be called by a child node
   * \param [localCoordinates] The Local Coordinates of the child node
   */
  Vec2 TranslateLocalToWorld(const Vec2 &localCoordinates);
  /**
   * Transforms World Coordinates into Coordinates relative to this position
   * This function should be called by a child node
   * \param [worldCoordinates] The World Coordinates of the child node
   */
  Vec2 TranslateWorldToLocal(const Vec2 &worldCoordinates);

  /**
   * Sets parent and sets Local Coordinates accordingly
   * without affecting World Coordinates
   */
  void SetParent(MapNode *parent);

  /// Returns the node at the top of the heirarchy (the node without a parent)
  MapNode *GetRootNode();

  // Data
  /// Pointer to parent
  MapNode *mParent;
  /// Local representation of position relative to parent
  Vec2 positionLocal;
  /// Absolute representation of position
  Vec2 positionWorld;
};

class ComponentPositionManager;
class ComponentPosition : public BaseComponent {
 public:
  ComponentPosition(EID id, MapNode *parent, ComponentPositionManager *manager);
  ~ComponentPosition();

  void Update();
  /// Override 'SetParent', MapNode needs to know when the parent has changed
  void SetParent(BaseComponent *p);

  /// Get Position Relative to Parent
  Vec2 GetPositionLocal();
  /// Get Absolute Position
  Vec2 GetPositionWorld();
  /// Get Movement Vector
  Vec2 GetMovement();
  /// Get Acceleration Vector
  Vec2 GetAcceleration();

  /// Sets local position
  void SetPositionLocal(Vec2 pos);
  /// Wrapper around SetPositionLocal
  void SetPositionLocalX(float x);
  /// Wrapper around SetPositionLocal
  void SetPositionLocalY(float y);

  /// Sets absolute position
  void SetPositionWorld(Vec2 pos);
  /// Wrapper around SetPositionWorld
  void SetPositionWorldX(float x);
  /// Wrapper around SetPositionWorld
  void SetPositionWorldY(float y);

  /// Sets Movement Speed
  void SetMovement(Vec2 mov);
  /// Wrapper around SetMovement
  void SetMovementX(float x);
  /// Wrapper around SetMovement
  void SetMovementY(float y);

  /// Sets Acceleration Speed
  void SetAcceleration(Vec2 acl);
  /// Wrapper around SetAcceleration
  void SetAccelerationX(float x);
  /// Wrapper around SetAcceleration
  void SetAccelerationY(float y);

  void SetMaxSpeed(float speed) { maximumSpeed = speed; }

  void IncrementPosition(Vec2 pos);
  void IncrementMovement(Vec2 mov);
  void IncrementAcceleration(Vec2 accel);

  Vec2 TranslateWorldToLocal(const Vec2 &world);
  Vec2 TranslateLocalToWorld(const Vec2 &local);

  MapNode *GetMapNode();

 private:
  Vec2 mMovement;
  Vec2 mAcceleration;
  float maximumSpeed;

  MapNode mNode;
};

class ComponentPositionManager
    : public BaseComponentManager_Impl<ComponentPosition> {
 public:
  ComponentPositionManager(EventDispatcher *e);
  ~ComponentPositionManager();

  std::unique_ptr<ComponentPosition> ConstructComponent(
      EID id, ComponentPosition *parent);

  MapNode *const GetRootNode();

 private:
  /**
   * This node is used as the root for all other mapNodes
   * All Position Component nodes are guaranteed to have a parent
   * if a Position Component doesn't have a parent itself, then the
   * component's node will have this node as a parent
   */
  MapNode mRootNode;
};

#endif
