#include "CompCollision.h"
#include "../Kernel.h"
#include "math.h"

TiledTileLayer *TColPacket::GetLayer() { return tl; }

RSC_Heightmap TColPacket::GetHmap() {
  if (tl->UsesHMaps() == false) {
    RSC_Heightmap l;
    return l;
  }
  GID id = tl->GetGID(tileX, tileY);
  TiledSet *ts = (TiledSet *)K_StateMan.GetCurrentState()
                     ->GetCurrentMap()
                     ->GetTiledData()
                     ->gid.GetItem(id);
  return ts->GetHeightMap(id);
}

ComponentCollision::ComponentCollision(EID id, ComponentPosition *pos,
                                       ComponentCollisionManager *manager)
    : mNextBoxID(0), BaseComponent(id, manager), myPos(pos) {}

ComponentCollision::~ComponentCollision() { myPos = NULL; }

void ComponentCollision::Update() {}

void ComponentCollisionManager::SetDependencies(ComponentPositionManager *pos) {
  dependencyPosition = pos;
}

CollisionBox *ComponentCollision::AddCollisionBox(const Shape *shape) {
  auto thisBoxID = mNextBoxID;
  mNextBoxID++;
  int defaultOrder = 0;

  mBoxes.emplace(std::pair<int, CollisionBox>(
      thisBoxID, CollisionBox(thisBoxID, defaultOrder, shape, myPos,
                              &mActiveTileBoxes, &mActiveEntityBoxes)));

  auto thisBoxIt = mBoxes.find(thisBoxID);

  /* this is ok because
   * References to elements in the unordered_map container remain valid in all
   * cases, even after a rehash.
   * See http://www.cplusplus.com/reference/unordered_map/unordered_map/insert/
   */
  return &(thisBoxIt->second);
}

void ComponentCollision::SetPrimaryCollisionBox(CollisionBox *box) {
  if (box != NULL) {
    if (GetPrimary() == NULL) {
      box->mPrimary = true;
    }
  } else {
    return;
  }
}

CollisionBox *ComponentCollision::GetColBox(int boxid) {
  auto it = mBoxes.find(boxid);
  if (it == mBoxes.end()) {
    std::stringstream ss;
    ss << "Couldn't find boxid " << boxid;

    LOG_WARN(ss.str());
    return NULL;
  }

  return &(it->second);
}

CollisionBox *ComponentCollision::GetPrimary() {
  for (auto i = mBoxes.begin(); i != mBoxes.end(); i++) {
    if (i->second.mPrimary) {
      return &(i->second);
    }
  }

  return NULL;
}

///////////////////////////////
// Component Collision Manager//
///////////////////////////////

ComponentCollisionManager::ComponentCollisionManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e) {}

std::unique_ptr<ComponentCollision>
ComponentCollisionManager::ConstructComponent(EID id,
                                              ComponentCollision *parent) {
  auto comp = std::make_unique<ComponentCollision>(
      id, dependencyPosition->GetComponent(id), this);
  return comp;
}

void ComponentCollisionManager::SendCollisionEvent(
    const ComponentCollision &sender, const ComponentCollision &reciever,
    CollisionBox *recieverBox, Event::MSG mes) {
  EColPacket ePacket;
  ePacket.name = "";     // sender.name;
  ePacket.objType = "";  // sender.objType;
  ePacket.box = recieverBox;

  EColPacket::ExtraDataDefinition extraData(&ePacket);
  Event event(sender.GetEID(), reciever.GetEID(), mes, "ENTITY_COLLISION",
              &extraData);
  mEventDispatcher->DispatchEvent(event);
}

void ComponentCollisionManager::UpdateCheckEntityCollision() {
  ComponentCollision *comp1 = NULL;
  ComponentCollision *comp2 = NULL;

  CollisionBox *primaryBox1;
  CollisionBox *primaryBox2;

  // These sets ensure only one collision is sent per box
  // if the other entity has multiple colboxes that collide with the first
  // entity's only collision box
  // these sets ensure that the first entity will only recieve one event for
  // it's only collision box
  // whereas the other will get a collision event for each of its boxes
  std::set<CollisionBox *> alreadyRegisteredBox1;
  std::set<CollisionBox *> alreadyRegisteredBox2;

  bool primaryPass;

  /* ISSUE
   * collision boxes can be part of multiple buckets
   * therefore, two eids can be compared twice
   */

  std::set<std::pair<EID, EID>> alreadyProcessed;
  for (auto bucketIt = grid.buckets.begin(); bucketIt != grid.buckets.end();
       bucketIt++) {
    for (auto eidIt1 = bucketIt->second.begin();
         eidIt1 != bucketIt->second.end(); eidIt1++) {
      comp1 = mComponentList[*eidIt1].get();

      for (auto eidIt2 = eidIt1 + 1; eidIt2 != bucketIt->second.end();
           eidIt2++) {
        auto newPair = std::pair<EID, EID>(*eidIt1, *eidIt2);
        if (alreadyProcessed.find(newPair) != alreadyProcessed.end()) {
          continue;
        }
        alreadyProcessed.insert(newPair);

        comp2 = mComponentList[*eidIt2].get();
        primaryPass = false;

        alreadyRegisteredBox1.clear();
        alreadyRegisteredBox2.clear();

        primaryBox1 = comp1->GetPrimary();
        primaryBox2 = comp2->GetPrimary();

        primaryBox1->UpdateWorldCoord();
        primaryBox2->UpdateWorldCoord();

        // Check first if the primaries collide
        if ((primaryBox1 != NULL) && (primaryBox2 != NULL)) {
          primaryPass = primaryBox1->Collides(primaryBox2).mCollided;
        }
        if (primaryPass) {
          for (auto boxIt1 = comp1->mActiveEntityBoxes.begin();
               boxIt1 != comp1->mActiveEntityBoxes.end(); boxIt1++) {
            auto box1 = (*boxIt1);

            for (auto boxIt2 = comp2->mActiveEntityBoxes.begin();
                 boxIt2 != comp2->mActiveEntityBoxes.end(); boxIt2++) {
              auto box2 = (*boxIt2);

              box1->UpdateWorldCoord();
              box2->UpdateWorldCoord();
              // Each entity will only be sent one collision event per
              // collision box max
              if (box1->Collides(box2).mCollided) {
                if (alreadyRegisteredBox1.find(box1) ==
                    alreadyRegisteredBox1.end()) {
                  SendCollisionEvent(*comp2, *comp1, box1,
                                     Event::MSG::COLLISION_ENTITY);
                  alreadyRegisteredBox1.insert(box1);
                }

                if (alreadyRegisteredBox2.find(box2) ==
                    alreadyRegisteredBox2.end()) {
                  SendCollisionEvent(*comp1, *comp2, box2,
                                     Event::MSG::COLLISION_ENTITY);
                  alreadyRegisteredBox2.insert(box2);
                }
              }
            }
          }
        }
      }
    }
  }
}

void ComponentCollisionManager::UpdateCheckTileCollision(RSC_Map *currentMap) {
  TiledTileLayer *tLayer = NULL;
  const Shape *shape;
  TColPacket packet;

  if (currentMap == NULL) {
    return;
  }

  for (auto compIt1 = mComponentList.begin(); compIt1 != mComponentList.end();
       compIt1++) {
    for (auto boxIt1 = compIt1->second->mActiveTileBoxes.begin();
         boxIt1 != compIt1->second->mActiveTileBoxes.end(); boxIt1++) {
      auto box1 = (*boxIt1);

      auto layerCallbacks = &box1->mLayersToCheck;
      if (layerCallbacks->empty()) {
        continue;
      }

      // Adds box coordinates to entity's coordinates
      box1->UpdateWorldCoord();
      shape = box1->GetWorldCoord();
      auto registerFirst = box1->mReturnOnlyFirstTileCollision;

      for (auto layerCallback = layerCallbacks->begin();
           layerCallback != layerCallbacks->end(); layerCallback++) {
        auto layer = std::get<0>(*layerCallback);
        auto callback = std::get<1>(*layerCallback);
        auto collisions = shape->Contains(layer).collisions;
        for (auto collision = collisions.begin(); collision != collisions.end();
             collision++) {
          packet.tileX = collision->x;
          packet.tileY = collision->y;
          packet.box = box1;
          packet.tl = layer;

          RegisterTileCollision(&packet, compIt1->first, callback);
          if (registerFirst) {
            goto breakout;
          }
        }
      }
    breakout:;
    }
  }
}

void ComponentCollisionManager::RegisterTileCollision(
    TColPacket *packet, EID id, CollisionBox::Callback callback) {
  TColPacket::ExtraDataDefinition extraData(packet);
  Event event(EID_SYSTEM, id, Event::MSG::COLLISION_TILE, "TILE", &extraData);
  mEventDispatcher->DispatchEvent(event);

  if (callback != NULL) {
    callback(packet);
  }
}

void ComponentCollisionManager::UpdateBuckets(int widthPixels) {
  grid.UpdateBuckets(&mComponentList, widthPixels);
}

void CollisionGrid::UpdateBuckets(
    const std::unordered_map<EID, std::unique_ptr<ComponentCollision>> *comps,
    int mapWidthPixels) {
  std::set<int> hashes;
  CollisionBox *primaryBox;
  const Shape *shape;

  // Collision will get WEIRD once outside the map
  // Collision will also fail if the objct is too large (larger than 3x3 grid

  buckets.clear();
  for (auto it = comps->cbegin(); it != comps->cend(); it++) {
    primaryBox = it->second->GetPrimary();
    if (primaryBox == NULL) {
      continue;
    }

    primaryBox->UpdateWorldCoord();
    shape = primaryBox->GetWorldCoord();
    Rect r = (static_cast<const Rect *>(shape))->Round();

    int left = r.GetLeft();
    int right = r.GetRight();
    int bottom = r.GetBottom();
    int top = r.GetTop();

    hashes.insert((left / COLLISION_GRID_SIZE) +
                  ((top / COLLISION_GRID_SIZE) * mapWidthPixels));
    hashes.insert((right / COLLISION_GRID_SIZE) +
                  ((top / COLLISION_GRID_SIZE) * mapWidthPixels));
    hashes.insert((left / COLLISION_GRID_SIZE) +
                  ((bottom / COLLISION_GRID_SIZE) * mapWidthPixels));
    hashes.insert((right / COLLISION_GRID_SIZE) +
                  ((bottom / COLLISION_GRID_SIZE) * mapWidthPixels));

    for (auto hashIt = hashes.begin(); hashIt != hashes.end(); hashIt++) {
      buckets[*hashIt].push_back(it->second->GetEID());
    }
    hashes.clear();
  }
}

void ComponentCollisionManager::Update() {
  AddNewComponents();
  GameStateManager *gs = &K_StateMan;
  if(gs == NULL){return;}
  GameState *state = gs->GetCurrentState();
  auto stateMap = state->GetCurrentMap();
  if (stateMap == NULL) {
    return;
  }

  // update grid
  UpdateBuckets(stateMap->GetWidthPixels());
  UpdateCheckEntityCollision();
  UpdateCheckTileCollision(K_StateMan.GetCurrentState()->GetCurrentMap());
}

//////////////
// EColPcaket//
//////////////
EColPacket::ExtraDataDefinition::ExtraDataDefinition(const EColPacket *p)
    : packet(p) {}

void EColPacket::ExtraDataDefinition::SetExtraData(Event *event) {
  event->extradata = packet;
}

const EColPacket *EColPacket::ExtraDataDefinition::GetExtraData(
    const Event *event) {
  return static_cast<const EColPacket *>(event->extradata);
}

//////////////
// TColPcaket//
//////////////
TColPacket::ExtraDataDefinition::ExtraDataDefinition(const TColPacket *p)
    : packet(p) {}

void TColPacket::ExtraDataDefinition::SetExtraData(Event *event) {
  event->extradata = packet;
}

const TColPacket *TColPacket::ExtraDataDefinition::GetExtraData(
    const Event *event) {
  return static_cast<const TColPacket *>(event->extradata);
}

void ComponentCollisionManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")

      .beginClass<TColPacket>("TColPacket")
      .addConstructor<void (*)(void)>()
      .addFunction("GetTileX", &TColPacket::GetTileX)
      .addFunction("GetTileY", &TColPacket::GetTileY)
      .addFunction("GetBox", &TColPacket::GetBox)
      .addFunction("GetLayer", &TColPacket::GetLayer)
      .addFunction("GetHmap", &TColPacket::GetHmap)
      .endClass()

      .beginClass<EColPacket>("EColPacket")
      .addConstructor<void (*)(void)>()
      .addFunction("GetName", &EColPacket::GetName)
      .addFunction("GetType", &EColPacket::GetType)
      .addFunction("GetBox", &EColPacket::GetBox)
      .endClass()

      .beginClass<RSC_Heightmap>("RSC_Heightmap")
      .addFunction("GetHeightMapH", &RSC_Heightmap::GetHeightMapH)
      .addFunction("GetHeightMapV", &RSC_Heightmap::GetHeightMapV)
      .addData("angleH", &RSC_Heightmap::angleH)
      .addData("angleV", &RSC_Heightmap::angleV)
      .endClass()

      .beginClass<Shape>("Shape")
      .addData("x", &Shape::x)  // Read-Write
      .addData("y", &Shape::y)  // Read-Write
      .endClass()

      .deriveClass<Rect, Shape>("Rect")
      .addConstructor<void (*)(float, float, float, float)>()
      .addData("w", &Rect::w)  // Read-Write
      .addData("h", &Rect::h)  // Read-Write
      .addFunction("GetTop", &Rect::GetTop)
      .addFunction("GetBottom", &Rect::GetBottom)
      .addFunction("GetLeft", &Rect::GetLeft)
      .addFunction("GetRight", &Rect::GetRight)
      .endClass()

      .beginClass<CollisionBox>("CollisionBox")
      .addFunction("Activate", &CollisionBox::Activate)
      .addFunction("Deactivate", &CollisionBox::Deactivate)
      .addFunction("GetID", &CollisionBox::GetID)
      .addFunction("CheckForEntities", &CollisionBox::CheckForEntities)
      .addFunction("CheckForTiles", &CollisionBox::CheckForTiles)
      .addFunction("SetShape", &CollisionBox::SetShape)
      .addFunction("SetOrder", &CollisionBox::SetOrder)
      .addFunction("RegisterEveryTileCollision",
                   &CollisionBox::RegisterEveryTileCollision)
      .addFunction("RegisterFirstTileCollision",
                   &CollisionBox::RegisterFirstTileCollision)
      .addFunction("CheckForLayer", &CollisionBox::CheckForLayerLuaInterface)
      .addFunction("CheckForLayers", &CollisionBox::CheckForLayersLuaInterface)
      .endClass()

      .beginClass<ComponentCollision>("ComponentCollision")
      .addFunction("SetPrimaryCollisionBox",
                   &ComponentCollision::SetPrimaryCollisionBox)
      .addFunction("AddCollisionBox", &ComponentCollision::AddCollisionBox)
      .endClass()

      .beginClass<ComponentCollisionManager>("ComponentCollisionManager")
      .endClass()
      .endNamespace();
}
