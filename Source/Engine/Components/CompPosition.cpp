#include "CompPosition.h"
#include "../Errorlog.h"
#include "math.h"

///////////
// MapNode//
///////////
MapNode::MapNode() { mParent = NULL; }
MapNode::~MapNode() {}

void MapNode::SetParent(MapNode *parent) {
  if (mParent == NULL) {
    mParent = parent;
    if (parent == NULL) {
      return;
    }

    positionLocal = mParent->TranslateWorldToLocal(positionLocal);
    return;
  }
  Coord2df oldParentCoord, newParentcoord;
  // Set local coordinates to world
  positionLocal = mParent->TranslateLocalToWorld(positionLocal);

  mParent = parent;
  if (parent == NULL) {
    return;
  }

  // Convert world coordinates back to local
  // This time local being relative to the new parent
  positionLocal = mParent->TranslateWorldToLocal(positionLocal);
}

void MapNode::UpdateWorld() {
  // Component manager will ensure that the parents are run before the children
  // This guarantees that the parent's world position is up-to-date

  if (mParent == NULL) {
    // Set World Coordinates to Local Coordinates
    positionWorld = positionLocal;
  }
  // If this node has a parent, translate the world Coordinates by their world
  // coordinates
  else {
    positionWorld = positionLocal + mParent->positionWorld;
  }
}

MapNode *MapNode::GetRootNode() {
  // If this node doesn't have a parent, it is the root
  if (mParent == NULL) {
    return this;
  }

  // Recurse until Root is found
  return mParent->GetRootNode();
}

Coord2df MapNode::TranslateLocalToWorld(const Coord2df &localCoordinates) {
  Coord2df worldCoordinates(localCoordinates);
  if (mParent == NULL) {
    return worldCoordinates;
  }

  // Translate coordinates by the local position of this node
  worldCoordinates = localCoordinates + positionLocal;

  // Translate coordinates by the local position of the parent node
  return mParent->TranslateLocalToWorld(worldCoordinates);
}

Coord2df MapNode::TranslateWorldToLocal(const Coord2df &worldCoordinates) {
  Coord2df localCoordinates(worldCoordinates);
  if (mParent == NULL) {
    return localCoordinates;
  }

  // Translate coordinates by the local position of this node
  localCoordinates = worldCoordinates - positionLocal;

  // Translate coordinates by the local position of the parent node
  return mParent->TranslateWorldToLocal(localCoordinates);
}

/////////////////////
// ComponentPosition//
/////////////////////
ComponentPosition::ComponentPosition(EID id, MapNode *parent,
                                     ComponentPositionManager *manager)
    : BaseComponent(id, manager) {
  mEntityID = id;
  maximumSpeed = 15.0;
  mNode.SetParent(parent);
}
ComponentPosition::~ComponentPosition() {}

void ComponentPosition::SetParent(BaseComponent *p) {
  parent = p;

  // If parent is null, then set the node parent to the root node owned by the
  // manager
  if (p == NULL) {
    mNode.SetParent(mNode.GetRootNode());
  }
  // Set node's parent to the parent's node
  else {
    auto parentPosition = static_cast<ComponentPosition *>(p);
    mNode.SetParent(parentPosition->GetMapNode());
  }
}

Coord2df ComponentPosition::GetPositionWorld() { return mNode.positionWorld; }

Coord2df ComponentPosition::GetPositionLocal() { return mNode.positionLocal; }

Coord2df ComponentPosition::GetMovement() { return mMovement; }

Coord2df ComponentPosition::GetAcceleration() { return mAcceleration; }

void ComponentPosition::Update() {
  // Clamp movement speed to maximum
  if (mMovement.x > maximumSpeed) {
    mMovement.x = maximumSpeed;
  } else if (mMovement.x < -maximumSpeed) {
    mMovement.x = -maximumSpeed;
  }
  if (mMovement.y > maximumSpeed) {
    mMovement.y = maximumSpeed;
  } else if (mMovement.y < -maximumSpeed) {
    mMovement.y = -maximumSpeed;
  }

  // Increment local position by movement speed
  mNode.positionLocal = mNode.positionLocal + mMovement;

  // Increment movement by acceleration
  mMovement = mMovement + mAcceleration;

  mNode.UpdateWorld();
}

void ComponentPosition::IncrementPosition(Coord2df pos) {
  mNode.positionLocal = mNode.positionLocal + pos;
  mNode.positionWorld = mNode.positionWorld + pos;
}
void ComponentPosition::IncrementMovement(Coord2df mov) {
  mMovement = mMovement + mov;
}
void ComponentPosition::IncrementAcceleration(Coord2df accel) {
  mAcceleration = mAcceleration + accel;
}

void ComponentPosition::SetPositionLocal(Coord2df pos) {
  mNode.positionLocal = pos;
  mNode.UpdateWorld();
}
void ComponentPosition::SetPositionLocalX(float x) {
  SetPositionLocal(Coord2df(x, mNode.positionLocal.y));
}
void ComponentPosition::SetPositionLocalY(float y) {
  SetPositionLocal(Coord2df(mNode.positionLocal.x, y));
}

void ComponentPosition::SetPositionWorld(Coord2df pos) {
  if (mNode.mParent == NULL) {
    mNode.positionWorld = pos;
    mNode.positionLocal = pos;
    return;
  }

  mNode.positionLocal = pos - mNode.mParent->positionWorld;

  mNode.UpdateWorld();
}
void ComponentPosition::SetPositionWorldX(float x) {
  SetPositionWorld(Coord2df(x, mNode.positionWorld.y));
}
void ComponentPosition::SetPositionWorldY(float y) {
  SetPositionWorld(Coord2df(mNode.positionWorld.x, y));
}

void ComponentPosition::SetMovement(Coord2df mov) { mMovement = mov; }
void ComponentPosition::SetMovementX(float x) {
  SetMovement(Coord2df(x, mMovement.y));
}
void ComponentPosition::SetMovementY(float y) {
  SetMovement(Coord2df(mMovement.x, y));
}

void ComponentPosition::SetAcceleration(Coord2df acl) { mAcceleration = acl; }
void ComponentPosition::SetAccelerationX(float x) {
  SetAcceleration(Coord2df(x, mAcceleration.y));
}
void ComponentPosition::SetAccelerationY(float y) {
  SetAcceleration(Coord2df(mAcceleration.x, y));
}

Coord2df ComponentPosition::TranslateWorldToLocal(const Coord2df &world) {
  // Node is guarnteed to have a parent, if the component doesn't have a parent
  // then the node's parent is set to the rootNode owned by the Manager
  return mNode.mParent->TranslateWorldToLocal(world);
}

Coord2df ComponentPosition::TranslateLocalToWorld(const Coord2df &local) {
  // Node is guarnteed to have a parent, if the component doesn't have a parent
  // then the node's parent is set to the rootNode owned by the Manager
  return mNode.mParent->TranslateLocalToWorld(local);
}

MapNode *ComponentPosition::GetMapNode() { return &mNode; }

////////////////////////////
// ComponentPositionManager//
////////////////////////////
ComponentPositionManager::ComponentPositionManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e) {}

ComponentPositionManager::~ComponentPositionManager() {}

std::unique_ptr<ComponentPosition> ComponentPositionManager::ConstructComponent(
    EID id, ComponentPosition *parent) {
  // Assign manager's root node as the node's parent by default
  // this is needed in case the parent is null,
  // in which case, the map node parent will be the root node
  auto pos = std::make_unique<ComponentPosition>(id, &mRootNode, this);

  // Change component's parent
  pos->SetParent(parent);

  return std::move(pos);
}

MapNode *const ComponentPositionManager::GetRootNode() { return &mRootNode; }
