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
  Coord2df TranslateLocalToWorld(const Coord2df &localCoordinates);
  /**
   * Transforms World Coordinates into Coordinates relative to this position
   * This function should be called by a child node
   * \param [worldCoordinates] The World Coordinates of the child node
   */
  Coord2df TranslateWorldToLocal(const Coord2df &worldCoordinates);

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
  Coord2df positionLocal;
  /// Absolute representation of position
  Coord2df positionWorld;
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
  Coord2df GetPositionLocal();
  /// Get Absolute Position
  Coord2df GetPositionWorld();
  /// Get Movement Vector
  Coord2df GetMovement();
  /// Get Acceleration Vector
  Coord2df GetAcceleration();

  /// Sets local position
  void SetPositionLocal(Coord2df pos);
  /// Wrapper around SetPositionLocal
  void SetPositionLocalX(float x);
  /// Wrapper around SetPositionLocal
  void SetPositionLocalY(float y);

  /// Sets absolute position
  void SetPositionWorld(Coord2df pos);
  /// Wrapper around SetPositionWorld
  void SetPositionWorldX(float x);
  /// Wrapper around SetPositionWorld
  void SetPositionWorldY(float y);

  /// Sets Movement Speed
  void SetMovement(Coord2df mov);
  /// Wrapper around SetMovement
  void SetMovementX(float x);
  /// Wrapper around SetMovement
  void SetMovementY(float y);

  /// Sets Acceleration Speed
  void SetAcceleration(Coord2df acl);
  /// Wrapper around SetAcceleration
  void SetAccelerationX(float x);
  /// Wrapper around SetAcceleration
  void SetAccelerationY(float y);

  void SetMaxSpeed(float speed) { maximumSpeed = speed; }

  void IncrementPosition(Coord2df pos);
  void IncrementMovement(Coord2df mov);
  void IncrementAcceleration(Coord2df accel);

  Coord2df TranslateWorldToLocal(const Coord2df &world);
  Coord2df TranslateLocalToWorld(const Coord2df &local);

  MapNode *GetMapNode();

 private:
  Coord2df mMovement;
  Coord2df mAcceleration;
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
