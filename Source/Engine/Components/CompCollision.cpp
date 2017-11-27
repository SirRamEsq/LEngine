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
    : BaseComponent(id, manager), myPos(pos) {}

ComponentCollision::~ComponentCollision() { myPos = NULL; }

void ComponentCollision::Update() {}

void ComponentCollisionManager::SetDependencies(ComponentPositionManager *pos) {
  dependencyPosition = pos;
}

void ComponentCollision::AddCollisionBox(const Shape *shape, int boxid,
                                         int orderNum) {
  boxes.insert(std::pair<int, CollisionBox>(
      boxid, CollisionBox(orderNum, 0, shape, myPos)));
  OrderList();
}

void ComponentCollision::SetShape(int boxid, const Shape *shape) {
  auto box = GetColBox(boxid);
  if (box != NULL) {
    if (shape != NULL) {
      box->SetShape(shape);
    } else {
      LOG_ERROR("ComponentCollision::SetShape passed null shape");
    }
  }
}

void ComponentCollision::SetPrimaryCollisionBox(int boxid) {
  CollisionBox *cb = GetColBox(boxid);
  if (cb != NULL) {
    cb->SetFlags((cb->Flags() | PRIMARY));
  } else {
    std::stringstream ss;
    ss << "Couldn't find box id: " << boxid;
    LOG_INFO(ss.str());
    return;
  }
  OrderList();
}

void ComponentCollision::CheckForEntities(int boxid) {
  CollisionBox *cb = GetColBox(boxid);
  if (cb != NULL) {
    cb->SetFlags((cb->Flags() | ENT_CHECK));
  } else {
    std::stringstream ss;
    ss << "Couldn't find box id: " << boxid;
    LOG_INFO(ss.str());
  }
}

void ComponentCollision::CheckForTiles(int boxid) {
  CollisionBox *cb = GetColBox(boxid);
  if (cb != NULL) {
    cb->SetFlags((cb->Flags() | TILE_CHECK));
  }
}

void ComponentCollision::Activate(int boxid) {
  CollisionBox *cb = GetColBox(boxid);
  if (cb != NULL) {
    cb->Activate();
  }
}
void ComponentCollision::Deactivate(int boxid) {
  CollisionBox *cb = GetColBox(boxid);
  if (cb != NULL) {
    cb->Deactivate();
  }
}

CollisionBox *ComponentCollision::GetColBox(int boxid) {
  auto it = boxes.find(boxid);
  if (it == boxes.end()) {
    return NULL;
  }

  return &(it->second);
}

CollisionBox *ComponentCollision::GetPrimary() {
  for (auto i = boxes.begin(); i != boxes.end(); i++) {
    if ((i->second.Flags() & PRIMARY)) {
      return &(i->second);
    }
  }

  return NULL;
}

void ComponentCollision::OrderList() {
  /*
  auto cmp = [](std::pair<int, CollisionBox> const &a,
                std::pair<int, CollisionBox> const &b) {
    return a.second < b.second;
  };
  // primary comes first,
  std::sort(boxes.begin(), boxes.end(), cmp);
  */
}

void ComponentCollision::CheckForLayer(int boxid, TiledTileLayer *layer,
                                       CollisionCallback callback) {
  auto boxCallbacks = mLayersToCheck.find(boxid);
  if (boxCallbacks == mLayersToCheck.end()) {
    // create new map if doesn't already exist
    mLayersToCheck[boxid] = LayerCallbacks();
    boxCallbacks = mLayersToCheck.find(boxid);
  }

  boxCallbacks->second[layer] = callback;
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
    int recieverBoxID, Event::MSG mes) {
  EColPacket ePacket;
  ePacket.name = sender.name;
  ePacket.objType = sender.objType;
  ePacket.box = recieverBoxID;

  EColPacket::ExtraDataDefinition extraData(&ePacket);
  Event event(sender.GetEID(), reciever.GetEID(), mes, "ENTITY_COLLISION",
              &extraData);
  eventDispatcher->DispatchEvent(event);
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
  std::set<int> alreadyRegisteredBox1;
  std::set<int> alreadyRegisteredBox2;

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
      comp1 = componentList[*eidIt1].get();

      for (auto eidIt2 = eidIt1 + 1; eidIt2 != bucketIt->second.end();
           eidIt2++) {
        auto newPair = std::pair<EID, EID>(*eidIt1, *eidIt2);
        if (alreadyProcessed.find(newPair) != alreadyProcessed.end()) {
          continue;
        }
        alreadyProcessed.insert(newPair);

        comp2 = componentList[*eidIt2].get();
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
          for (auto boxIt1 = comp1->GetItBeg(); boxIt1 != comp1->GetItEnd();
               boxIt1++) {  // iterate through the collision boxes of compIt1
            if ((boxIt1->second.Flags() & ENT_CHECK) != ENT_CHECK) {
              continue;
            }  // Skip if you the box doesn't have the ENT_CHECK flag set
            if (not boxIt1->second.IsActive()) {
              continue;
            }

            for (auto boxIt2 = comp2->GetItBeg(); boxIt2 != comp2->GetItEnd();
                 boxIt2++) {  // iterate through the collision boxes of compIt2
              if ((boxIt2->second.Flags() & ENT_CHECK) != ENT_CHECK) {
                continue;
              }
              if (not boxIt2->second.IsActive()) {
                continue;
              }

              boxIt1->second.UpdateWorldCoord();
              boxIt2->second.UpdateWorldCoord();
              if (boxIt1->second.Collides(&(boxIt2->second)).mCollided) {
                // Each entity will only be sent one collision event per
                // collision box max
                if (alreadyRegisteredBox1.find(boxIt1->first) ==
                    alreadyRegisteredBox1.end()) {
                  //                      Sender
                  //                      Reciever Reciever BoxID
                  SendCollisionEvent(*comp2, *comp1, boxIt1->first,
                                     Event::MSG::COLLISION_ENTITY);
                  // Register box to make sure an event isn't sent here again
                  alreadyRegisteredBox1.insert(boxIt1->first);
                }

                if (alreadyRegisteredBox2.find(boxIt2->first) ==
                    alreadyRegisteredBox2.end()) {
                  //                      Sender
                  //                      Reciever Reciever BoxID
                  SendCollisionEvent(*comp1, *comp2, boxIt2->first,
                                     Event::MSG::COLLISION_ENTITY);
                  // Register box to make sure an event isn't sent here again
                  alreadyRegisteredBox2.insert(boxIt2->first);
                }
              }
            }
          }
        }
      }
    }
  }
}

void ComponentCollision::CheckForLayerLuaInterface(int boxid,
                                                   TiledLayerGeneric *layer,
                                                   luabridge::LuaRef func) {
  if (layer == NULL) {
    LOG_ERROR("Layer is NULL");
    return;
  }
  if (func.isNil()) {
    LOG_ERROR("Func is Nil");
    return;
  }

  if (layer->layerType != LAYER_TILE) {
    LOG_ERROR("Layer passed was not a TILE layer");
    return;
  }
  auto tileLayer = (TiledTileLayer *)layer;
  auto funcWrapper = [func](TColPacket *packet) { func(packet); };
  CheckForLayer(boxid, tileLayer, funcWrapper);
}

void ComponentCollisionManager::UpdateCheckTileCollision(
    RSC_Map *currentMap) {
  // Put event into smart pointer so that the same event can be reused (not
  // multiple events allocated and deallocated on the stack)
  // May want to change this behaviour at some point, as recievers of the event
  // may expect that they can hold on to it
  Coord2df ul(0, 0), dr(0, 0);
  TiledTileLayer *tLayer = NULL;
  const Shape *shape;
  TColPacket packet;  // for messaging purposes

  if (currentMap == NULL) {
    return;
  }  // No point in checking if there's no map to check against
  auto solidLayers = currentMap->GetSolidTileLayers();

  for (auto compIt1 = componentList.begin(); compIt1 != componentList.end();
       compIt1++) {
    // Start iterating through collision boxes
    // boxIt1 is a colbox iterator for each collision component
    // i iterates through all of the collision components
    for (auto boxIt1 = compIt1->second.get()->GetItBeg();
         boxIt1 != compIt1->second.get()->GetItEnd(); boxIt1++) {
      if ((boxIt1->second.Flags() & TILE_CHECK) != TILE_CHECK) {
        continue;
      }
      if (not boxIt1->second.IsActive()) {
        continue;
      }

      // Add layers for each box to check for
      std::vector<TiledTileLayer *> layers = solidLayers;
      auto layersToCheck = compIt1->second->mLayersToCheck;
      auto boxCallbacks = layersToCheck.find(boxIt1->first);
      if (boxCallbacks != layersToCheck.end()) {
        for (auto i = boxCallbacks->second.begin();
             i != boxCallbacks->second.end(); i++) {
          layers.push_back(i->first);
        }
      }

      // Adds box coordinates to entity's coordinates
      boxIt1->second.UpdateWorldCoord();
      shape = boxIt1->second.GetWorldCoord();
      // only rects for now
      const Rect *absoluteCoordinates = (static_cast<const Rect *>(shape));
      Rect r = (static_cast<const Rect *>(shape))->Round();

      ul.x = r.GetLeft();
      ul.y = r.GetTop();
      dr.x = r.GetRight();
      dr.y = r.GetBottom();
      int txx1 = ul.x;
      int txx2 = dr.x;
      int tyy1 = ul.y;
      int tyy2 = dr.y;

      CoordToGrid(txx1, tyy1);
      CoordToGrid(txx2, tyy2);
      if ((txx1 == txx2) and
          (tyy1 == tyy2)) {  // if the top left is the same as the bottom right,
                             // then the whole box has fit inside a single tile
        tLayer = GetTileLayerCollision(&layers, txx1, tyy1);

        if (tLayer == NULL) {
          continue;
        }

        packet.tl = tLayer;
        packet.tileX = txx1;
        packet.tileY = tyy2;
        packet.posX = absoluteCoordinates->GetLeft();
        packet.posY = absoluteCoordinates->GetTop();
        packet.box = boxIt1->first;

        ComponentCollision::CollisionCallback callback = NULL;
        if (boxCallbacks != layersToCheck.end()) {
          auto callbackIterator = boxCallbacks->second.find(tLayer);
          if (callbackIterator != boxCallbacks->second.end()) {
            callback = callbackIterator->second;
          }
        }
        RegisterTileCollision(&packet, compIt1->first, callback);

        continue;
      }

      int differenceX =
          txx2 - txx1;  // Both differences will always be positive
      int differenceY = tyy2 - tyy1;

      bool negativeH = (r.GetOriginHorizontal() == Shape::Origin::Right);
      bool negativeW = (r.GetOriginVertical() == Shape::Origin::Bottom);
      int tx, ty;

      if (negativeW) {
        tx = txx2;
      } else {
        tx = txx1;
      }
      if (negativeH) {
        ty = tyy2;
      } else {
        ty = tyy1;
      }

      bool breakOut = false;
      for (int iter = 0; iter <= differenceX; iter++) {
        for (int iter2 = 0; iter2 <= differenceY; iter2++) {
          tLayer = GetTileLayerCollision(&layers, tx, ty);
          if (tLayer != NULL) {
            packet.tileX = tx;
            packet.tileY = ty;
            packet.posX = absoluteCoordinates->GetLeft();
            packet.posY = absoluteCoordinates->GetTop();
            packet.box = boxIt1->first;
            packet.tl = tLayer;

            ComponentCollision::CollisionCallback callback = NULL;
            if (boxCallbacks != layersToCheck.end()) {
              auto callbackIterator = boxCallbacks->second.find(tLayer);
              if (callbackIterator != boxCallbacks->second.end()) {
                callback = callbackIterator->second;
              }
            }

            RegisterTileCollision(&packet, compIt1->first, callback);

            breakOut = true;
          }
          if (!negativeH) {
            ty += 1;
          } else {
            ty -= 1;
          }
          if (breakOut) {
            break;
          }
        }
        if (negativeH) {
          ty = tyy2;
        } else {
          ty = tyy1;
        }
        if (!negativeW) {
          tx += 1;
        } else {
          tx -= 1;
        }
        if (breakOut) {
          break;
        }
      }
    }
  }
}

void ComponentCollisionManager::RegisterTileCollision(
    TColPacket *packet, EID id,
    ComponentCollision::CollisionCallback callback) {
  TColPacket::ExtraDataDefinition extraData(packet);
  Event event(EID_SYSTEM, id, Event::MSG::COLLISION_TILE, "TILE", &extraData);
  eventDispatcher->DispatchEvent(event);

  if (callback != NULL) {
    callback(packet);
  }
}

void ComponentCollisionManager::UpdateBuckets(int widthPixels) {
  grid.UpdateBuckets(&componentList, widthPixels);
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
  GameStateManager *gs = &K_StateMan;
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

TiledTileLayer *ComponentCollisionManager::GetTileLayerCollision(
    const std::vector<TiledTileLayer *> *layers, unsigned int x,
    unsigned int y) {
  for (auto i = layers->begin(); i != layers->end(); i++) {
    auto id = (*i)->HasTile(x, y);
    if (id != 0) {
      return *i;
    }
  }
  return NULL;
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
