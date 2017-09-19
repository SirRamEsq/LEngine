#include "../catch.hpp"
#include "../../Engine/Components/CompCollision.h"
#include "../mocks/StateManager.h"
#include "../mocks/ComponentManagerMock.h"
#include "../../Engine/EntityManager.h"
#include "../../Engine/Event.h"
#include "../../Engine/Defines.h"


TEST_CASE("EntiyManager Creation and Deletion Test", "[EntityManager]"){
	GameStateManager_Mock dummyManager(NULL);
	EventDispatcher eventDispatcher;
	ComponentTestManager compMan(&eventDispatcher);
	EntityManager entityMan(&dummyManager);
	entityMan.RegisterComponentManager(&compMan, 10);

	EID eid1 = entityMan.NewEntity();
	EID eid2 = entityMan.NewEntity();
	EID eid3 = entityMan.NewEntity();

	REQUIRE(entityMan.GetEntityCount() == 3);

	compMan.AddComponent(eid1);
	compMan.AddComponent(eid2);
	compMan.AddComponent(eid3);

	compMan.Update();

	auto compPointer = static_cast<ComponentTest*>(compMan.GetComponent(eid1));
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = static_cast<ComponentTest*>(compMan.GetComponent(eid2));
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = static_cast<ComponentTest*>(compMan.GetComponent(eid3));
	REQUIRE( compPointer->GetUpdateCount() == 1);

	entityMan.DeleteEntity(eid2);

	//Make sure entiyMan has deleted the entity
	REQUIRE(entityMan.GetEntityCount() == 2);

	//Make sure componentManager still has it until cleanup is called
	auto testPointer = compMan.GetComponent(eid2);
	REQUIRE( testPointer != NULL);

	entityMan.Cleanup();

	testPointer = compMan.GetComponent(eid2);
	REQUIRE( testPointer == NULL);

	compMan.Update();
	compPointer = static_cast<ComponentTest*>(compMan.GetComponent(eid1));
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = static_cast<ComponentTest*>(compMan.GetComponent(eid3));
	REQUIRE( compPointer->GetUpdateCount() == 2);

	//SECTION("Delete one Entity"){

	//}
}

