#include "../catch.hpp"
#include "../../Source/Components/CompCollision.h"
#include "../mocks/RSC_MapMock.h"
#include "../../Source/EntityManager.h"
#include "../../Source/Event.h"
#include "../../Source/Defines.h"

TEST_CASE("Test RSC_Map Mock and Tile Layer", "[collision][rsc_map]"){
    RSC_MapMock testMap("TEST", 64, 64);
    REQUIRE(NULL == testMap.GetTileLayerCollision(2, 2, true));

    //set GID to 1 at 2,2
    testMap.layer.SetGID(2,2,1);
    REQUIRE(NULL != testMap.GetTileLayerCollision(2, 2, true));
}
int callbackValue = 0;

void CallbackFunction(const Event* event){
    if(event->message==MSG_TILE_COLLISION){
        callbackValue++;
    }
}

TEST_CASE("COMP Collision", "[collision][rsc_map]"){
    REQUIRE(0 == callbackValue);

    RSC_MapMock testMap("TEST", 64, 64);

    //set TILE to 1 at 2,2
    testMap.layer.SetGID(2,2,1);
    //set position to be collided with at tile 2,2
    Coord2df pos(2 * LENGINE_DEF_TILE_W, 2 * LENGINE_DEF_TILE_H);


    EventDispatcher             eventDispatcher;
    EntityManager               entityMan;

    eventDispatcher.SetDependencies(NULL, &entityMan);

    ComponentCollisionManager   collisionManager(&eventDispatcher);
    ComponentPositionManager    positionManager(&eventDispatcher);

    entityMan.RegisterComponentManager(&positionManager,         EntityManager::DEFAULT_UPDATE_ORDER::POSITION);
    entityMan.RegisterComponentManager(&collisionManager,        EntityManager::DEFAULT_UPDATE_ORDER::COLLISION);

    EID entity = EID_MIN;

    collisionManager.dependencyPosition = &positionManager;

    positionManager.AddComponent(entity);
    collisionManager.AddComponent(entity);

    auto compCol = (ComponentCollision*) collisionManager.GetComponent(entity);
    auto compPos = (ComponentPosition*) positionManager.GetComponent(entity);

    compPos->SetPositionLocal(pos);
    compCol->SetEventCallbackFunction(CallbackFunction);

    CRect box(-1,-1, 1, 1);
    int boxID = 0;
    compCol->AddCollisionBox(box, boxID);
    compCol->CheckForTiles(boxID);
    positionManager.Update();

    collisionManager.UpdateCheckTileCollision(&testMap);

    REQUIRE(1 == callbackValue);
}
