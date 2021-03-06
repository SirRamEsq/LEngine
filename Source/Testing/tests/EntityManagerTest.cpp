#include "../../Engine/EntityManager.h"
#include "../../Engine/Components/CompCollision.h"
#include "../../Engine/Defines.h"
#include "../../Engine/Event.h"
#include "../catch.hpp"
#include "../mocks/ComponentManagerMock.h"
#include "../mocks/StateManager.h"

TEST_CASE("EntiyManager Creation and Deletion Test", "[EntityManager]") {
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
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 1);

  entityMan.DeleteEntity(eid2);

  // Make sure entiyMan has deleted the entity
  REQUIRE(entityMan.GetEntityCount() == 2);

  // Make sure componentManager still has it until cleanup is called
  auto testPointer = compMan.GetComponent(eid2);
  REQUIRE(testPointer != NULL);

  entityMan.Cleanup();

  testPointer = compMan.GetComponent(eid2);
  REQUIRE(testPointer == NULL);

  compMan.Update();
  compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 2);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 2);
}

TEST_CASE("EntityManager Name Lookup", "[EntityManager]") {
  GameStateManager_Mock dummyManager(NULL);
  EventDispatcher eventDispatcher;
  ComponentTestManager compMan(&eventDispatcher);
  EntityManager entityMan(&dummyManager);
  entityMan.RegisterComponentManager(&compMan, 10);

  std::string ent1 = "ent1";
  std::string ent2 = "ent2";
  std::string ent3 = "ent3";
  // same name as ent1
  std::string entDuplicate = "ent1";

  EID eid1 = entityMan.NewEntity(ent1);
  EID eid2 = entityMan.NewEntity(ent2);
  EID eid3 = entityMan.NewEntity(ent3);
  EID eidDuplicate = entityMan.NewEntity(entDuplicate);

  REQUIRE(entityMan.GetEntityCount() == 4);

  auto eidVector = entityMan.NameLookup(ent1);
  REQUIRE(eidVector.size() == 2);
  auto testValue = eidVector[0];
  REQUIRE(eid1 == testValue);
  testValue = eidVector[1];
  REQUIRE(eidDuplicate == testValue);

  eidVector = entityMan.NameLookup(ent2);
  REQUIRE(eidVector.size() == 1);
  testValue = eidVector[0];
  REQUIRE(eid2 == testValue);

  eidVector = entityMan.NameLookup(ent3);
  REQUIRE(eidVector.size() == 1);
  testValue = eidVector[0];
  REQUIRE(eid3 == testValue);
}

TEST_CASE("EntityManager set delete all", "[EntityManager][regression]") {
  GameStateManager_Mock dummyManager(NULL);
  EventDispatcher eventDispatcher;
  ComponentTestManager compMan(&eventDispatcher);
  EntityManager entityMan(&dummyManager);
  entityMan.RegisterComponentManager(&compMan, 10);

  // Call cleanup with no entities
  entityMan.Cleanup();

  // Ask entityMan to delete all entities, setting the mFlagDelete all to true
  entityMan.ClearAllEntities();
  // Call Cleanup to delete all entities (Even though there are none)
  entityMan.Cleanup();

  // Create two entities
  EID eid1 = entityMan.NewEntity();
  EID eid2 = entityMan.NewEntity();
  // Call cleanup again
  // this shouldn't affect anything, since no entities have been deleted
  entityMan.Cleanup();

  // run regular unit test
  EID eid3 = entityMan.NewEntity();

  REQUIRE(entityMan.GetEntityCount() == 3);

  compMan.AddComponent(eid1);
  compMan.AddComponent(eid2);
  compMan.AddComponent(eid3);

  compMan.Update();

  auto compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 1);

  entityMan.DeleteEntity(eid2);

  // Make sure entiyMan has deleted the entity
  REQUIRE(entityMan.GetEntityCount() == 2);

  // Make sure componentManager still has it until cleanup is called
  auto testPointer = compMan.GetComponent(eid2);
  REQUIRE(testPointer != NULL);

  entityMan.Cleanup();

  testPointer = compMan.GetComponent(eid2);
  REQUIRE(testPointer == NULL);

  compMan.Update();
  compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 2);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 2);

  // Test to make sure ClearAllEntities actuall clears all active entities
  auto eid4 = entityMan.NewEntity();
  auto eid5 = entityMan.NewEntity();
  auto eid6 = entityMan.NewEntity();
  compMan.AddComponent(eid4);
  compMan.AddComponent(eid5);
  compMan.AddComponent(eid6);
  compMan.Update();
  REQUIRE(entityMan.GetEntityCount() == 5);

  entityMan.ClearAllEntities();
  entityMan.Cleanup();
  REQUIRE(entityMan.GetEntityCount() == 0);

  testPointer = compMan.GetComponent(eid6);
  REQUIRE(testPointer == NULL);
}

TEST_CASE("EntityManager Deactivation/Activation testing", "[EntityManager]") {
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
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 1);

  std::vector<EID> eidList;
  eidList.push_back(eid2);
  entityMan.Deactivate(eidList);

  compMan.Update();

  compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 2);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 2);

  eidList.push_back(eid1);
  entityMan.Deactivate(eidList);

  compMan.Update();

  compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 2);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 1);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 3);

  entityMan.Activate(eidList);

  compMan.Update();

  compPointer = compMan.GetComponent(eid1);
  REQUIRE(compPointer->GetUpdateCount() == 3);
  compPointer = compMan.GetComponent(eid2);
  REQUIRE(compPointer->GetUpdateCount() == 2);
  compPointer = compMan.GetComponent(eid3);
  REQUIRE(compPointer->GetUpdateCount() == 4);
}

TEST_CASE("Break EntityCount", "[EntityManager]") {
  GameStateManager_Mock dummyManager(NULL);
  EntityManager entityMan(&dummyManager);
  auto eid1 = entityMan.NewEntity();
  auto eid2 = entityMan.NewEntity();
  auto eid3 = entityMan.NewEntity();
  REQUIRE(entityMan.GetEntityCount() == 3);

  //this entity is valid, try to delete it three times
  entityMan.DeleteEntity(eid1);
  entityMan.DeleteEntity(eid1);
  entityMan.DeleteEntity(eid1);
  //even though cleanup isn't called, they're still considered to be deleted
  REQUIRE(entityMan.GetEntityCount() == 2);

  //these eids don't exist, shouldn't reduce entityCount
  entityMan.DeleteEntity(31337);
  entityMan.DeleteEntity(eid3 + 1);
  entityMan.DeleteEntity(entityMan.NewEntity());
  REQUIRE(entityMan.GetEntityCount() == 2);

}
