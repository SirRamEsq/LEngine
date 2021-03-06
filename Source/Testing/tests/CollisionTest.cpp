#include "../../Engine/Components/CompCollision.h"
#include "../../Engine/Defines.h"
#include "../../Engine/EntityManager.h"
#include "../../Engine/Event.h"
#include "../catch.hpp"
#include "../mocks/RSC_MapMock.h"
#include "../mocks/StateManager.h"

TEST_CASE("Test RSC_Map Mock and Tile Layer", "[collision][rsc_map]") {
  RSC_MapMock testMap("TEST", 64, 64);
  REQUIRE(NULL == testMap.GetTileLayerCollision(2, 2, true));

  // set GID to 1 at 2,2
  testMap.layer.SetGID(2, 2, 1);
  REQUIRE(NULL != testMap.GetTileLayerCollision(2, 2, true));
}
int callbackValue = 0;
int callbackValueEntity = 0;

void CallbackFunction(const Event *event) {
  if (event->message == Event::MSG::COLLISION_TILE) {
    callbackValue++;
  }
  if (event->message == Event::MSG::COLLISION_ENTITY) {
    callbackValueEntity++;
  }
}

CollisionBox *gCollisionBox;
void CallbackFunctionDeactivate(const Event *event) {
  gCollisionBox->Deactivate();
}

TEST_CASE("Tile Collision tests with mock map", "[collision][rsc_map]") {
  GameStateManager_Mock dummyManager(NULL);
  callbackValue = 0;
  REQUIRE(0 == callbackValue);

  RSC_MapMock testMap("TEST", 64, 64);
  int solidX = 2;
  int solidY = 2;
  // set TILE to 1 at 2,2
  testMap.layer.SetGID(solidX, solidY, 1);
  // set position to be collided with at tile 2,2
  Vec2 pos(2 * LENGINE_DEF_TILE_W, 2 * LENGINE_DEF_TILE_H);

  EventDispatcher eventDispatcher;
  EntityManager entityMan(&dummyManager);

  eventDispatcher.SetDependencies(NULL, &entityMan);

  ComponentCollisionManager collisionManager(&eventDispatcher);
  ComponentPositionManager positionManager(&eventDispatcher);

  entityMan.RegisterComponentManager(
      &positionManager, EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
  entityMan.RegisterComponentManager(
      &collisionManager, EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);

  EID entity = EID_MIN;

  collisionManager.dependencyPosition = &positionManager;

  positionManager.AddComponent(entity);
  collisionManager.AddComponent(entity);
  positionManager.AddNewComponents();
  collisionManager.AddNewComponents();

  auto compCol = collisionManager.GetComponent(entity);
  auto compPos = positionManager.GetComponent(entity);

  compPos->SetPositionLocal(pos);
  compCol->SetEventCallbackFunction(CallbackFunction);

  int boxw = 1;
  int boxh = 1;
  Rect box(0, 0, boxw, boxh);
  auto cbox = compCol->AddCollisionBox(&box);
  cbox->CheckForTiles();

  auto callback = [](TColPacket *packet) { return; };
  auto solidLayers = testMap.GetSolidTileLayers();
  cbox->CheckForLayer(&solidLayers, callback);
  positionManager.Update();

  SECTION("Basic test") {
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);

    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(2 == callbackValue);

    pos.x -= 16;
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(2 == callbackValue);
  }

  SECTION("Activate / Deactivate") {
    cbox->Activate();
    cbox->Deactivate();
    cbox->Deactivate();
    cbox->Activate();
    cbox->Activate();
    cbox->Activate();
	cbox->ToggleActivation();
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);

    cbox->Deactivate();
	cbox->ToggleActivation();
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);
  }

  SECTION("Out of bounds test") {
    pos.x = 30000;
    pos.y = 30000;
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);

    pos.x = -30000;
    pos.y = -30000;
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);
  }

  SECTION("Collision register boundaries") {
    //////////////
    // RIGHT_SIDE//
    //////////////
    // Should not register a collision, just outside area
    pos.x = (solidX * 16) - boxw - 1;
    pos.y = (solidY * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);

    // Should register a collision
    pos.x = (solidX * 16) - boxw;
    pos.y = (solidY * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);

    /////////////
    // LEFT_SIDE//
    /////////////
    callbackValue = 0;
    // Should not register a collision, just outside area
    pos.x = ((solidX + 1) * 16);
    pos.y = (solidY * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);

    // Should register a collision
    pos.x = ((solidX + 1) * 16) - 1;
    pos.y = ((solidY)*16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);

    ////////////
    // TOP_SIDE//
    ////////////
    callbackValue = 0;
    // Should not register a collision, just outside area
    pos.y = ((solidY + 1) * 16);
    pos.x = (solidX * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);

    // Should register a collision
    pos.y = ((solidY + 1) * 16) - 1;
    pos.x = ((solidX)*16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);

    ///////////////
    // BOTTOM_SIDE//
    ///////////////
    callbackValue = 0;
    // Should not register a collision, just outside area
    pos.y = (solidY * 16) - boxh - 1;
    pos.x = (solidX * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(0 == callbackValue);

    // Should register a collision
    pos.y = (solidY * 16) - boxh;
    pos.x = (solidX * 16);
    compPos->SetPositionLocal(pos);
    collisionManager.UpdateCheckTileCollision(&testMap);
    REQUIRE(1 == callbackValue);
  }
  callbackValue = 0;
}

TEST_CASE("Entity Collision tests", "[collision]") {
  GameStateManager_Mock dummyManager(NULL);
  callbackValueEntity = 0;

  EventDispatcher eventDispatcher;
  EntityManager entityMan(&dummyManager);

  eventDispatcher.SetDependencies(NULL, &entityMan);

  ComponentCollisionManager collisionManager(&eventDispatcher);
  ComponentPositionManager positionManager(&eventDispatcher);

  entityMan.RegisterComponentManager(
      &positionManager, EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
  entityMan.RegisterComponentManager(
      &collisionManager, EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);
  collisionManager.dependencyPosition = &positionManager;

  EID entity = EID_MIN;
  // setup 5 entities
  for (int i = 0; i < 5; i++) {
    Vec2 pos(16 * i, 8 * i);

    positionManager.AddComponent(entity);
    collisionManager.AddComponent(entity);
    positionManager.AddNewComponents();
    collisionManager.AddNewComponents();
    auto compCol = collisionManager.GetComponent(entity);
    auto compPos = positionManager.GetComponent(entity);

    compPos->SetPositionLocal(pos);
    compCol->SetEventCallbackFunction(CallbackFunction);

    Rect box(0, 0, 8, 8);
    auto cbox = compCol->AddCollisionBox(&box);
    cbox->CheckForEntities();
    compCol->SetPrimaryCollisionBox(cbox);

    entity++;
  }
  positionManager.Update();

  RSC_MapMock testMap("TEST", 2560, 2560);

  SECTION("Basic test") {
    collisionManager.UpdateBuckets(testMap.GetWidthPixels());
    collisionManager.UpdateCheckEntityCollision();
    REQUIRE(0 == callbackValueEntity);

    auto compPos = positionManager.GetComponent(EID_MIN);
    Vec2 pos(16, 8);
    compPos->SetPositionLocal(pos);
    auto compCol = collisionManager.GetComponent(entity);

    collisionManager.UpdateBuckets(testMap.GetWidthPixels());
    collisionManager.UpdateCheckEntityCollision();

    // 2 collisions, both colliding entities will register 1 collision
    REQUIRE(2 == callbackValueEntity);
  }

  SECTION("Deactivate mid-frame test") {
    collisionManager.UpdateBuckets(testMap.GetWidthPixels());
    collisionManager.UpdateCheckEntityCollision();
    auto compPos = positionManager.GetComponent(EID_MIN);
    Vec2 pos(16, 8);
    compPos->SetPositionLocal(pos);

    auto compCol = collisionManager.GetComponent(EID_MIN);
    compCol->SetEventCallbackFunction(CallbackFunctionDeactivate);
	Rect box(0,0,32,32);
    auto cbox = compCol->AddCollisionBox(&box);
    cbox->CheckForEntities();
	gCollisionBox = cbox;
    collisionManager.UpdateBuckets(testMap.GetWidthPixels());

	//callback will deactivate a cbox while iterating through the cbox list
	//this should not crash
    collisionManager.UpdateCheckEntityCollision();
	gCollisionBox = NULL;
  }

  /**
   * This test will check to see if boxes that are larger than buckets will be
   * processed correctly
   */
  SECTION("Large boxes test") {
    collisionManager.UpdateBuckets(testMap.GetWidthPixels());
    collisionManager.UpdateCheckEntityCollision();
    REQUIRE(0 == callbackValueEntity);

    // two large entities overlapping
    for (int i = 0; i < 2; i++) {
      Vec2 pos(COLLISION_GRID_SIZE * 10, COLLISION_GRID_SIZE * 10);

      positionManager.AddComponent(entity);
      collisionManager.AddComponent(entity);
      positionManager.AddNewComponents();
      collisionManager.AddNewComponents();
      auto compCol = collisionManager.GetComponent(entity);
      auto compPos = positionManager.GetComponent(entity);

      compPos->SetPositionLocal(pos);
      compCol->SetEventCallbackFunction(CallbackFunction);

      Rect box(0, 0, COLLISION_GRID_SIZE * 2, COLLISION_GRID_SIZE * 2);
      auto cbox = compCol->AddCollisionBox(&box);
      cbox->CheckForEntities();
      compCol->SetPrimaryCollisionBox(cbox);

      entity++;
    }

    collisionManager.UpdateBuckets(testMap.GetWidthPixels());
    collisionManager.UpdateCheckEntityCollision();

    // 2 collisions, both colliding entities will register 1 collision
    REQUIRE(2 == callbackValueEntity);
  }
}
