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
  Vec2 oldParentCoord, newParentcoord;
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

Vec2 MapNode::TranslateLocalToWorld(const Vec2 &localCoordinates) {
  Vec2 worldCoordinates(localCoordinates);
  if (mParent == NULL) {
    return worldCoordinates;
  }

  // Translate coordinates by the local position of this node
  worldCoordinates = localCoordinates + positionLocal;

  // Translate coordinates by the local position of the parent node
  return mParent->TranslateLocalToWorld(worldCoordinates);
}

Vec2 MapNode::TranslateWorldToLocal(const Vec2 &worldCoordinates) {
  Vec2 localCoordinates(worldCoordinates);
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

Vec2 ComponentPosition::GetPositionWorld() { return mNode.positionWorld; }

Vec2 ComponentPosition::GetPositionLocal() { return mNode.positionLocal; }

Vec2 ComponentPosition::GetMovement() { return mMovement; }

Vec2 ComponentPosition::GetAcceleration() { return mAcceleration; }

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

void ComponentPosition::IncrementPosition(Vec2 pos) {
  mNode.positionLocal = mNode.positionLocal + pos;
  mNode.positionWorld = mNode.positionWorld + pos;
}
void ComponentPosition::IncrementMovement(Vec2 mov) {
  mMovement = mMovement + mov;
}
void ComponentPosition::IncrementAcceleration(Vec2 accel) {
  mAcceleration = mAcceleration + accel;
}

void ComponentPosition::SetPositionLocal(Vec2 pos) {
  mNode.positionLocal = pos;
  mNode.UpdateWorld();
}
void ComponentPosition::SetPositionLocalX(float x) {
  SetPositionLocal(Vec2(x, mNode.positionLocal.y));
}
void ComponentPosition::SetPositionLocalY(float y) {
  SetPositionLocal(Vec2(mNode.positionLocal.x, y));
}

void ComponentPosition::SetPositionWorld(Vec2 pos) {
  if (mNode.mParent == NULL) {
    mNode.positionWorld = pos;
    mNode.positionLocal = pos;
    return;
  }

  mNode.positionLocal = pos - mNode.mParent->positionWorld;

  mNode.UpdateWorld();
}
void ComponentPosition::SetPositionWorldX(float x) {
  SetPositionWorld(Vec2(x, mNode.positionWorld.y));
}
void ComponentPosition::SetPositionWorldY(float y) {
  SetPositionWorld(Vec2(mNode.positionWorld.x, y));
}

void ComponentPosition::SetMovement(Vec2 mov) { mMovement = mov; }
void ComponentPosition::SetMovementX(float x) {
  SetMovement(Vec2(x, mMovement.y));
}
void ComponentPosition::SetMovementY(float y) {
  SetMovement(Vec2(mMovement.x, y));
}

void ComponentPosition::SetAcceleration(Vec2 acl) { mAcceleration = acl; }
void ComponentPosition::SetAccelerationX(float x) {
  SetAcceleration(Vec2(x, mAcceleration.y));
}
void ComponentPosition::SetAccelerationY(float y) {
  SetAcceleration(Vec2(mAcceleration.x, y));
}

Vec2 ComponentPosition::TranslateWorldToLocal(const Vec2 &world) {
  // Node is guarnteed to have a parent, if the component doesn't have a parent
  // then the node's parent is set to the rootNode owned by the Manager
  return mNode.mParent->TranslateWorldToLocal(world);
}

Vec2 ComponentPosition::TranslateLocalToWorld(const Vec2 &local) {
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

Vec2 ComponentPositionManager::GetWorld(EID id) {
  ComponentPosition *pos = GetComponent(id);
  if (pos == NULL) {
    return Vec2(0, 0);
  }
  return pos->GetPositionWorld();
}
Vec2 ComponentPositionManager::GetMovement(EID id) {
  ComponentPosition *pos = GetComponent(id);
  if (pos == NULL) {
    return Vec2(0, 0);
  }
  return pos->GetMovement();
}

MapNode *const ComponentPositionManager::GetRootNode() { return &mRootNode; }
void ComponentPositionManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")

      .beginClass<BaseComponentManager_Impl<ComponentPosition>>(
          "BASE_COMP_IMPL_POS")
      .addFunction("SetParent",
                   &BaseComponentManager_Impl<ComponentPosition>::SetParent)
      .endClass()

      .deriveClass<ComponentPositionManager,
                   BaseComponentManager_Impl<ComponentPosition>>(
          "ComponentPositionManager")
      .addFunction("GetWorld", &ComponentPositionManager::GetWorld)
      .addFunction("GetMovement", &ComponentPositionManager::GetMovement)
      .endClass()

      .deriveClass<ComponentPosition, BaseComponent>("ComponentPosition")
      .addFunction("GetPositionLocal", &ComponentPosition::GetPositionLocal)
      .addFunction("GetPositionWorld", &ComponentPosition::GetPositionWorld)
      .addFunction("GetMovement", &ComponentPosition::GetMovement)
      .addFunction("GetAcceleration", &ComponentPosition::GetAcceleration)

      .addFunction("SetPositionLocal", &ComponentPosition::SetPositionLocal)
      .addFunction("SetPositionLocalX", &ComponentPosition::SetPositionLocalX)
      .addFunction("SetPositionLocalY", &ComponentPosition::SetPositionLocalY)

      .addFunction("SetPositionWorld", &ComponentPosition::SetPositionWorld)
      .addFunction("SetPositionWorldX", &ComponentPosition::SetPositionWorldX)
      .addFunction("SetPositionWorldY", &ComponentPosition::SetPositionWorldY)

      .addFunction("SetMovement", &ComponentPosition::SetMovement)
      .addFunction("SetMovementX", &ComponentPosition::SetMovementX)
      .addFunction("SetMovementY", &ComponentPosition::SetMovementY)

      .addFunction("SetAcceleration", &ComponentPosition::SetAcceleration)
      .addFunction("SetAccelerationX", &ComponentPosition::SetAccelerationX)
      .addFunction("SetAccelerationY", &ComponentPosition::SetAccelerationY)

      .addFunction("SetMaxSpeed", &ComponentPosition::SetMaxSpeed)

      .addFunction("IncrementPosition", &ComponentPosition::IncrementMovement)
      .addFunction("IncrementMovement", &ComponentPosition::IncrementMovement)
      .addFunction("IncrementAcceleration",
                   &ComponentPosition::IncrementAcceleration)

      .addFunction("TranslateWorldToLocal",
                   &ComponentPosition::TranslateWorldToLocal)
      .addFunction("TranslateLocalToWorld",
                   &ComponentPosition::TranslateLocalToWorld)
      .endClass()
      .endNamespace();
}
