#include "../catch.hpp"
#include "../../Engine/Components/CompCollision.h"
#include "../mocks/RSC_MapMock.h"
#include "../mocks/StateManager.h"
#include "../../Engine/EntityManager.h"
#include "../../Engine/Event.h"
#include "../../Engine/Defines.h"

GameStateManager dummyManager;
TEST_CASE("Test RSC_Map Mock and Tile Layer", "[collision][rsc_map]"){
    RSC_MapMock testMap("TEST", 64, 64);
    REQUIRE(NULL == testMap.GetTileLayerCollision(2, 2, true));

    //set GID to 1 at 2,2
    testMap.layer.SetGID(2,2,1);
    REQUIRE(NULL != testMap.GetTileLayerCollision(2, 2, true));
}
int callbackValue = 0;
int callbackValueEntity = 0;

void CallbackFunction(const Event* event){
    if(event->message== Event::MSG::COLLISION_TILE){
        callbackValue++;
    }
    if(event->message== Event::MSG::COLLISION_ENTITY){
        callbackValueEntity++;
    }
}

TEST_CASE("Tile Collision tests with mock map", "[collision][rsc_map]"){
    REQUIRE(0 == callbackValue);

    RSC_MapMock testMap("TEST", 64, 64);
	int solidX = 2;
	int solidY = 2;
    //set TILE to 1 at 2,2
    testMap.layer.SetGID(solidX,solidY,1);
    //set position to be collided with at tile 2,2
    Coord2df pos(2 * LENGINE_DEF_TILE_W, 2 * LENGINE_DEF_TILE_H);


    EventDispatcher             eventDispatcher;
    EntityManager               entityMan(&dummyManager);

    eventDispatcher.SetDependencies(NULL, &entityMan);

    ComponentCollisionManager   collisionManager(&eventDispatcher);
    ComponentPositionManager    positionManager(&eventDispatcher);

    entityMan.RegisterComponentManager(&positionManager, EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
    entityMan.RegisterComponentManager(&collisionManager,EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);

    EID entity = EID_MIN;

    collisionManager.dependencyPosition = &positionManager;

    positionManager.AddComponent(entity);
    collisionManager.AddComponent(entity);

    auto compCol = (ComponentCollision*) collisionManager.GetComponent(entity);
    auto compPos = (ComponentPosition*) positionManager.GetComponent(entity);

    compPos->SetPositionLocal(pos);
    compCol->SetEventCallbackFunction(CallbackFunction);

    int boxw = 1;
    int boxh = 1;
    CRect box(0,0, boxw, boxh);
    int boxID = 0;
    compCol->AddCollisionBox(box, boxID);
    compCol->CheckForTiles(boxID);
    positionManager.Update();

    SECTION("Basic test"){
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(1 == callbackValue);

        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(2 == callbackValue);

        pos.x-=16;
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(2 == callbackValue);
    }

    SECTION("Out of bounds test"){
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

    SECTION("Collision register boundaries"){
		//////////////
		//RIGHT_SIDE//
		//////////////
		//Should not register a collision, just outside area
        pos.x = (solidX * 16) - boxw - 1;
        pos.y = (solidY * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(0 == callbackValue);

        //Should register a collision
        pos.x = (solidX * 16) - boxw;
        pos.y = (solidY * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(1 == callbackValue);

		/////////////
		//LEFT_SIDE//
		/////////////
		callbackValue=0;
		//Should not register a collision, just outside area
        pos.x = ((solidX+1) * 16);
        pos.y = (solidY * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(0 == callbackValue);

        //Should register a collision
        pos.x = ((solidX+1) * 16) - 1;
        pos.y = ((solidY) * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(1 == callbackValue);

		////////////
		//TOP_SIDE//
		////////////
		callbackValue=0;
		//Should not register a collision, just outside area
        pos.y = ((solidY+1) * 16);
        pos.x = (solidX * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(0 == callbackValue);

        //Should register a collision
        pos.y = ((solidY+1) * 16) - 1;
        pos.x = ((solidX) * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(1 == callbackValue);

		///////////////
		//BOTTOM_SIDE//
		///////////////
		callbackValue=0;
		//Should not register a collision, just outside area
        pos.y = (solidY * 16) - boxh - 1;
        pos.x = (solidX * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(0 == callbackValue);

        //Should register a collision
        pos.y = (solidY * 16) - boxh;
        pos.x = (solidX * 16);
        compPos->SetPositionLocal(pos);
        collisionManager.UpdateCheckTileCollision(&testMap);
        REQUIRE(1 == callbackValue);
    }
    callbackValue=0;
}

TEST_CASE("Entity Collision tests", "[collision]"){
    REQUIRE(0 == callbackValueEntity);

    EventDispatcher             eventDispatcher;
    EntityManager               entityMan(&dummyManager);

    eventDispatcher.SetDependencies(NULL, &entityMan);

    ComponentCollisionManager   collisionManager(&eventDispatcher);
    ComponentPositionManager    positionManager(&eventDispatcher);

    entityMan.RegisterComponentManager(&positionManager, EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
    entityMan.RegisterComponentManager(&collisionManager,EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);
    collisionManager.dependencyPosition = &positionManager;

    EID entity = EID_MIN;
    //setup 5 entities
    for(int i = 0; i < 5; i++){
        Coord2df pos(16 * i, 8 * i);

        positionManager.AddComponent(entity);
        collisionManager.AddComponent(entity);
        auto compCol = (ComponentCollision*) collisionManager.GetComponent(entity);
        auto compPos = (ComponentPosition*) positionManager.GetComponent(entity);

        compPos->SetPositionLocal(pos);
        compCol->SetEventCallbackFunction(CallbackFunction);

        CRect box(0,0, 8, 8);
        int boxID = 0;
        compCol->AddCollisionBox(box, boxID);
        compCol->CheckForEntities(boxID);
        compCol->SetPrimaryCollisionBox(boxID);

        entity++;
    }
    positionManager.Update();

    RSC_MapMock testMap("TEST", 256,256);
    SECTION("Basic test"){
        collisionManager.UpdateBuckets(testMap.GetWidthPixels());
        collisionManager.UpdateCheckEntityCollision();
        REQUIRE(0 == callbackValueEntity);

        auto compPos = (ComponentPosition*) positionManager.GetComponent(EID_MIN);
        Coord2df pos(16,8);
        compPos->SetPositionLocal(pos);

        collisionManager.UpdateBuckets(testMap.GetWidthPixels());
        collisionManager.UpdateCheckEntityCollision();

        //2 collisions, both colliding entities will register 1 collision
        REQUIRE(2 == callbackValueEntity);
    }
}
