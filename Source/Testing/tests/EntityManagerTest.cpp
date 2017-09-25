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

	auto compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid3);
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
	compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 2);
}

TEST_CASE("EntityManager Name Lookup", "[EntityManager]"){
	GameStateManager_Mock dummyManager(NULL);
	EventDispatcher eventDispatcher;
	ComponentTestManager compMan(&eventDispatcher);
	EntityManager entityMan(&dummyManager);
	entityMan.RegisterComponentManager(&compMan, 10);

	std::string ent1 = "ent1";
	std::string ent2 = "ent2";
	std::string ent3 = "ent3";
	//same name as ent1
	std::string entWrong = "ent1";

	EID eid1 = entityMan.NewEntity(ent1);
	EID eid2 = entityMan.NewEntity(ent2);
	EID eid3 = entityMan.NewEntity(ent3);
	EID eidWrong = entityMan.NewEntity(entWrong);

	REQUIRE(entityMan.GetEntityCount() == 4);

	auto testValue = entityMan.GetEIDFromName(ent1);
	REQUIRE(eid1 == testValue);
	testValue = entityMan.GetEIDFromName(ent2);
	REQUIRE(eid2 == testValue);
	testValue = entityMan.GetEIDFromName(ent3);
	REQUIRE(eid3 == testValue);
	//If a name is already taken, it should not be overwritten
	testValue = entityMan.GetEIDFromName(entWrong);
	REQUIRE(eidWrong != testValue);
}

TEST_CASE("EntityManager set mFlagDelete all when no entities have been created", "[EntityManager][regression]"){
	GameStateManager_Mock dummyManager(NULL);
	EventDispatcher eventDispatcher;
	ComponentTestManager compMan(&eventDispatcher);
	EntityManager entityMan(&dummyManager);
	entityMan.RegisterComponentManager(&compMan, 10);

	//Call cleanup with no entities
	entityMan.Cleanup();

	//Ask entityMan to delete all entities, setting the mFlagDelete all to true
	entityMan.ClearAllEntities();
	//Call Cleanup to delete all entities (Even though there are none)
	entityMan.Cleanup();

	//Create two entities
	EID eid1 = entityMan.NewEntity();
	EID eid2 = entityMan.NewEntity();
	//Call cleanup again
	//this shouldn't affect anything, since no entities have been deleted
	entityMan.Cleanup();

	//run regular unit test
	EID eid3 = entityMan.NewEntity();

	REQUIRE(entityMan.GetEntityCount() == 3);

	compMan.AddComponent(eid1);
	compMan.AddComponent(eid2);
	compMan.AddComponent(eid3);

	compMan.Update();

	auto compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid3);
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
	compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 2);

}

TEST_CASE("EntityManager Deactivation/Activation testing", "[EntityManager]"){
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

	auto compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 1);

	std::vector<EID> eidList;
	eidList.push_back(eid2);
	entityMan.Deactivate(eidList);

	compMan.Update();

	compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 2);

	eidList.push_back(eid1);
	entityMan.Deactivate(eidList);

	compMan.Update();

	compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 1);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 3);

	entityMan.Activate(eidList);

	compMan.Update();

	compPointer = compMan.GetComponent(eid1);
	REQUIRE( compPointer->GetUpdateCount() == 3);
	compPointer = compMan.GetComponent(eid2);
	REQUIRE( compPointer->GetUpdateCount() == 2);
	compPointer = compMan.GetComponent(eid3);
	REQUIRE( compPointer->GetUpdateCount() == 4);
}
