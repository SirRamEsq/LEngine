#include "CollisionBox.h"
#include "CompPosition.h"

#include "../Resources/RSC_Map.h"
#include <algorithm>

bool SortCollisionBoxes(const CollisionBox *a, const CollisionBox *b) {
  return (*a) < (*b);
};

CollisionBox::CollisionBox(int id, int order, const Shape *shape,
                           ComponentPosition *pos, CB_Vector *activeTile,
                           CB_Vector *activeEntity)
    : mOrder(order),
      mBoxID(id),
      mActiveTileBoxes(activeTile),
      mActiveEntityBoxes(activeEntity),
      mPos(pos) {
  mPrimary = false;
  mReturnOnlyFirstTileCollision = true;
  mCheckTiles = false;
  mCheckEntities = false;
  Activate();
  SetShape(shape);
}

void CollisionBox::SetShape(const Shape *shape) {
  if (shape == NULL) {
    return;
  }
  mShape.reset(shape->MakeCopy().release());
  mShapeToWorldCoords.reset(shape->MakeCopy().release());
}

void CollisionBox::RegisterFirstTileCollision() {
  mReturnOnlyFirstTileCollision = true;
}
void CollisionBox::RegisterEveryTileCollision() {
  mReturnOnlyFirstTileCollision = false;
}

bool CollisionBox::operator<(const CollisionBox &rhs) const {
  if ((mPrimary) and (rhs.mPrimary)) {
    if (mOrder >= rhs.mOrder) {
      return true;
    } else {
      return false;
    }
  }
  if (mPrimary) {
    return true;
  }
  if (rhs.mPrimary) {
    return false;
  }

  if (mOrder >= rhs.mOrder) {
    return true;
  } else {
    return false;
  }

  return true;
}

const Shape *CollisionBox::GetWorldCoord() { return mShapeToWorldCoords.get(); }

void CollisionBox::UpdateWorldCoord() {
  if ((mShapeToWorldCoords.get() == NULL) or (mShape.get() == NULL)) {
    return;
  }
  auto wPos = mPos->GetPositionWorld();
  mShapeToWorldCoords->x = mShape->x + wPos.x;
  mShapeToWorldCoords->y = mShape->y + wPos.y;
}

CollisionResponse CollisionBox::Collides(const CollisionBox *box) {
  if (mShapeToWorldCoords.get() == NULL) {
    return CollisionResponse(0, 0, false);
  }
  auto result = mShapeToWorldCoords->Contains(box->mShapeToWorldCoords.get());
  return result;
}

void CollisionBox::Activate() {
  if (mActive) {
    return;
  }
  mActive = true;
  if (mCheckTiles) {
    AddSelfToVector(mActiveTileBoxes);
  }
  if (mCheckEntities) {
    AddSelfToVector(mActiveEntityBoxes);
  }
}

void CollisionBox::Deactivate() {
  mActive = false;
  if (mCheckTiles) {
    RemoveSelfFromVector(mActiveTileBoxes);
  }
  if (mCheckEntities) {
    RemoveSelfFromVector(mActiveEntityBoxes);
  }
}

void CollisionBox::CheckForEntities() {
  mCheckEntities = true;
  if (mActive) {
    AddSelfToVector(mActiveEntityBoxes);
  }
}
void CollisionBox::CheckForTiles() {
  mCheckTiles = true;
  if (mActive) {
    AddSelfToVector(mActiveTileBoxes);
  }
}

bool CollisionBox::IsActive() const { return mActive; }
bool CollisionBox::IsPrimary() const { return mPrimary; }

void CollisionBox::SetOrder(int order) { mOrder = order; }
void CollisionBox::CheckForLayerLuaInterface(TiledLayerGeneric *layer,
                                             luabridge::LuaRef func) {
  if (layer == NULL) {
    LOG_ERROR("Layer is NULL");
    return;
  }
  if (func.isNil()) {
    LOG_ERROR("Second Argument is Nil");
    return;
  }
  if (!func.isFunction()) {
    LOG_ERROR("Second argument is not a function!");
    return;
  }

  if (layer->layerType != LAYER_TILE) {
    LOG_ERROR("Layer passed was not a TILE layer");
    return;
  }
  auto tileLayer = (TiledTileLayer *)layer;
  auto funcWrapper = [func](TColPacket *packet) { func(packet); };
  CheckForLayer(tileLayer, funcWrapper);
}

void CollisionBox::CheckForLayersLuaInterface(
    std::vector<TiledLayerGeneric *> *layers, luabridge::LuaRef func) {
  if (func.isNil()) {
    LOG_ERROR("Second Argument is Nil");
    return;
  }
  if (!func.isFunction()) {
    LOG_ERROR("Second argument is not a function!");
    return;
  }

  auto funcWrapper = [func](TColPacket *packet) { func(packet); };
  for (auto i = layers->begin(); i != layers->end(); i++) {
    auto layer = *i;
    if (layer == NULL) {
      LOG_ERROR("Layer is NULL");
      continue;
    }
    if (layer->layerType != LAYER_TILE) {
      LOG_ERROR("Layer passed was not a TILE layer");
      continue;
    }
    auto tileLayer = (TiledTileLayer *)layer;
    CheckForLayer(tileLayer, funcWrapper);
  }
}

void CollisionBox::CheckForLayer(TiledTileLayer *layer,
                                 CollisionBox::Callback callback) {
  // function doesn't check to see if the layer already exists
  auto newLayerCallback =
      (std::pair<TiledTileLayer *, Callback>(layer, callback));

  mLayersToCheck.push_back(newLayerCallback);
}
void CollisionBox::CheckForLayer(std::vector<TiledTileLayer *> *layers,
                                 CollisionBox::Callback callback) {
  for (auto l = layers->begin(); l != layers->end(); l++) {
    CheckForLayer(*l, callback);
  }
}
void CollisionBox::AddSelfToVector(CB_Vector *vec) {
  for (auto i = vec->begin(); i != vec->end(); i++) {
    CollisionBox *box = (*i);
    if (box == this) {
	  return;
    }
  }
  vec->push_back(this);
  std::sort(vec->begin(), vec->end(), SortCollisionBoxes);
}
void CollisionBox::RemoveSelfFromVector(CB_Vector *vec) {
  if (vec->empty()) {
    return;
  }
  for (auto i = vec->begin(); i != vec->end(); i++) {
    CollisionBox *box = (*i);
    if (box == this) {
      vec->erase(i);
	  return;
    }
  }
}
