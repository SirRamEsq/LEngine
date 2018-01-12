#include "EntityManager.h"
#include "Kernel.h"
#include "StateManager.h"

EntityManager::EntityManager(GameStateManager *gsm) : mStateManager(gsm) {
  ASSERT(gsm != NULL);
  maxInUseEID = EID_MIN;
}

void EntityManager::DeleteEntity(EID id) {
  // if the id is alive and not alerady deleted
  if (aliveEntities.find(id) != aliveEntities.end()) {
    deadEntities.insert(id);
  }
}

unsigned int EntityManager::GetEntityCount() {
  return aliveEntities.size() - deadEntities.size();
}

void EntityManager::DispatchEvent(const Event *event) {
  for (auto i = componentsRegistered.begin(); i != componentsRegistered.end();
       i++) {
    i->second->HandleEvent(event);
  }
}

void EntityManager::BroadcastEvent(const Event *event) {
  // Send to state
  mStateManager->HandleEvent(event);
  for (auto i = componentsRegistered.begin(); i != componentsRegistered.end();
       i++) {
    i->second->BroadcastEvent(event);
  }
}

void EntityManager::Cleanup() {
  if (deadEntities.empty()) {
    return;
  }

  for (auto i = deadEntities.begin(); i != deadEntities.end(); i++) {
    EID id = *i;

    // Let all entities know that this entity is about to be deleted
    Event event(id, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "[DELETED]");
    BroadcastEvent(&event);

    for (auto i = componentsRegistered.begin(); i != componentsRegistered.end();
         i++) {
      i->second->DeleteComponent(id);
    }

    if (EIDToName.find(id) != EIDToName.end()) {
      std::string entityName = EIDToName[id];
      EIDToName.erase(id);
      nameToEID.erase(entityName);
    }

    reclaimedEIDs.push_back(id);
    aliveEntities.erase(id);
  }

  deadEntities.clear();
}

void EntityManager::RegisterComponentManager(BaseComponentManager *manager,
                                             int order) {
  if (componentsRegistered.find(order) != componentsRegistered.end()) {
    std::stringstream ss;
    ss << "Couldn't register component with order: '" << order
       << "' order id already taken.";
    throw Exception(ss.str());
  }

  // Assert that this order not already taken
  ASSERT(componentsRegistered.find(order) == componentsRegistered.end());

  componentsRegistered[order] = manager;
}

void EntityManager::SetName(EID id, const std::string &name) {}

EID EntityManager::NewEntity(const std::string &entityName) {
  auto eid = NewEntity();
  MapNameToEID(eid, entityName);
  return eid;
}

EID EntityManager::NewEntity() {
  EID newEntityID = 0;

  // First check if there are any reclaimed eids to use
  if (!reclaimedEIDs.empty()) {
    newEntityID = reclaimedEIDs.back();
    reclaimedEIDs.pop_back();
  }
  // If there are no old eids to use, make a new one
  else {
    newEntityID = maxInUseEID;
    maxInUseEID++;
  }

  aliveEntities.insert(newEntityID);
  activeEntities.insert(newEntityID);
  // Increment number of living entities

  return newEntityID;
}

void EntityManager::MapNameToEID(EID eid, const std::string &entityName) {
  if (entityName != "") {
    // Stop if in debug mode, this shouldn't happen
    ASSERT(nameToEID.find(entityName) == nameToEID.end());
    // Defensive programming if not in debug mode
    if (nameToEID.find(entityName) == nameToEID.end()) {
      nameToEID[entityName] = eid;
      EIDToName[eid] = entityName;
    }
  }
}

void EntityManager::ClearAllEntities() {
  for (EID i = EID_MIN; i < maxInUseEID; i++) {
    DeleteEntity(i);
  }
}

void EntityManager::ClearAllReservedEntities() {
  for (EID i = 0; i < EID_MIN; i++) {
    DeleteEntity(i);
  }
}

void EntityManager::ClearNameMappings() {
  nameToEID.clear();
  EIDToName.clear();
}

EID EntityManager::GetEIDFromName(const std::string &name) const {
  std::map<std::string, EID>::const_iterator i = nameToEID.find(name);
  if (i == nameToEID.end()) {
    return 0;
  }

  return i->second;
}

void EntityManager::SetParent(EID child, EID parent) {
  for (auto i = componentsRegistered.begin(); i != componentsRegistered.end();
       i++) {
    i->second->SetParent(child, parent);
  }
}

void EntityManager::ActivateAll() {
  for (auto i : inactiveEntities) {
    activeEntities.insert(i);
    for (auto comp : componentsRegistered) {
      comp.second->ActivateComponent(i);
    }
  }
  inactiveEntities.clear();
}

void EntityManager::DeactivateAll() {
  for (auto i : activeEntities) {
    inactiveEntities.insert(i);
    for (auto comp : componentsRegistered) {
      comp.second->DeactivateComponent(i);
    }
  }
  activeEntities.clear();
}

void EntityManager::ActivateAllExcept(const std::vector<EID> &entities) {
  if (entities.empty()) {
    ActivateAll();
    return;
  }

  std::vector<EID> toActivate;
  for (auto i : inactiveEntities) {
    bool skip = false;
    for (auto except : entities) {
      if (i == except) {
        skip = true;
        break;
      }
    }

    if (!skip) {
      toActivate.push_back(i);
    }
  }
  Activate(toActivate);
}
void EntityManager::DeactivateAllExcept(const std::vector<EID> &entities) {
  if (entities.empty()) {
    DeactivateAll();
    return;
  }

  std::vector<EID> toDeactivate;
  for (auto i : activeEntities) {
    bool skip = false;
    for (auto except : entities) {
      if (i == except) {
        skip = true;
        break;
      }
    }

    if (!skip) {
      toDeactivate.push_back(i);
    }
  }
  Deactivate(toDeactivate);
}

void EntityManager::Activate(const std::vector<EID> &entities) {
  for (auto i : entities) {
    Activate(i);
  }
}

void EntityManager::Deactivate(const std::vector<EID> &entities) {
  for (auto i : entities) {
    Deactivate(i);
  }
}

void EntityManager::Activate(EID id) {
  // Check if EID is inactive
  auto iActive = inactiveEntities.find(id);
  if (iActive != inactiveEntities.end()) {
    for (auto comp = componentsRegistered.begin();
         comp != componentsRegistered.end(); comp++) {
      comp->second->ActivateComponent(id);
      activeEntities.insert(id);
      inactiveEntities.erase(id);
    }
  }
}
void EntityManager::Deactivate(EID id) {
  // Check if EID is active
  auto iActive = activeEntities.find(id);
  if (iActive != activeEntities.end()) {
    for (auto comp = componentsRegistered.begin();
         comp != componentsRegistered.end(); comp++) {
      comp->second->DeactivateComponent(id);
      inactiveEntities.insert(id);
      activeEntities.erase(id);
    }
  }
}

void EntityManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")
      .beginClass<EntityManager>("EntityManager")
      .addFunction("Activate",
                   (void (EntityManager::*)(const std::vector<EID> &)) &
                       EntityManager::Activate)
      .addFunction("Deactivate",
                   (void (EntityManager::*)(const std::vector<EID> &)) &
                       EntityManager::Deactivate)

      .addFunction("ActivateAll", &EntityManager::ActivateAll)
      .addFunction("DeactivateAll", &EntityManager::DeactivateAll)

      .addFunction("ActivateAllExcept", &EntityManager::ActivateAllExcept)
      .addFunction("DeactivateAllExcept", &EntityManager::DeactivateAllExcept)

      .addFunction("GetEntityCount", &EntityManager::GetEntityCount)
      .addFunction("SetParent", &EntityManager::SetParent)

      .addFunction("Delete", &EntityManager::DeleteEntity)

      .addFunction("New", (EID(EntityManager::*)()) & EntityManager::NewEntity)
      .addFunction("MapNameToEID", &EntityManager::MapNameToEID)
      .endClass()
      .endNamespace();
}
