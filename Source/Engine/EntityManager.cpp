#include "EntityManager.h"
#include "Kernel.h"
#include "StateManager.h"

EntityManager::EntityManager(GameStateManager *gsm) : mStateManager(gsm) {
  ASSERT(gsm != NULL);
  maxInUseEID = EID_MIN;
}

void EntityManager::DeleteEntity(EID id) {
  // if the id is alive and not alerady deleted
  if (mAliveEntities.find(id) != mAliveEntities.end()) {
    mDeadEntities.insert(id);
  }
}

unsigned int EntityManager::GetEntityCount() {
  return mAliveEntities.size() - mDeadEntities.size();
}

void EntityManager::DispatchEvent(const Event *event) {
  for (auto i = mComponentsRegistered.begin(); i != mComponentsRegistered.end();
       i++) {
    i->second->HandleEvent(event);
  }
}

void EntityManager::BroadcastEvent(const Event *event) {
  // Send to state
  mStateManager->HandleEvent(event);
  for (auto i = mComponentsRegistered.begin(); i != mComponentsRegistered.end();
       i++) {
    i->second->BroadcastEvent(event);
  }
}

void EntityManager::Cleanup() {
  if (mDeadEntities.empty()) {
    return;
  }

  for (auto i = mDeadEntities.begin(); i != mDeadEntities.end(); i++) {
    EID id = *i;

    // Let all entities know that this entity is about to be deleted
    Event event(id, EID_ALLOBJS, Event::MSG::ENTITY_DELETED, "[DELETED]");
    BroadcastEvent(&event);

    for (auto i = mComponentsRegistered.begin();
         i != mComponentsRegistered.end(); i++) {
      i->second->DeleteComponent(id);
    }

    mReclaimedEIDs.push_back(id);
    mAliveEntities.erase(id);
  }

  mDeadEntities.clear();
}

void EntityManager::RegisterComponentManager(BaseComponentManager *manager,
                                             int order) {
  if (mComponentsRegistered.find(order) != mComponentsRegistered.end()) {
    std::stringstream ss;
    ss << "Couldn't register component with order: '" << order
       << "' order id already taken.";
    throw Exception(ss.str());
  }

  // Assert that this order not already taken
  ASSERT(mComponentsRegistered.find(order) == mComponentsRegistered.end());

  mComponentsRegistered[order] = manager;
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
  if (!mReclaimedEIDs.empty()) {
    newEntityID = mReclaimedEIDs.back();
    mReclaimedEIDs.pop_back();
  }
  // If there are no old eids to use, make a new one
  else {
    newEntityID = maxInUseEID;
    maxInUseEID++;
  }

  mAliveEntities.insert(newEntityID);
  mActiveEntities.insert(newEntityID);
  // Increment number of living entities

  return newEntityID;
}

void EntityManager::MapNameToEID(EID eid, const std::string &entityName) {
  if (entityName != "") {
    // Stop if in debug mode, this shouldn't happen
    ASSERT(mNameToEID.find(entityName) == mNameToEID.end());

    // Create new vector if needed
    if (mNameToEID.find(entityName) == mNameToEID.end()) {
      mNameToEID[entityName] = std::vector<EID>();
    }
    mNameToEID[entityName].push_back(eid);
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

void EntityManager::ClearNameMappings() { mNameToEID.clear(); }

std::vector<EID> EntityManager::NameLookup(const std::string &name) const {
  auto i = mNameToEID.find(name);
  if (i == mNameToEID.end()) {
    return std::vector<EID>();
  }

  return i->second;
}

void EntityManager::SetParent(EID child, EID parent) {
  for (auto i = mComponentsRegistered.begin(); i != mComponentsRegistered.end();
       i++) {
    i->second->SetParent(child, parent);
  }
}

void EntityManager::ActivateAll() {
  for (auto i : mInActiveEntities) {
    mActiveEntities.insert(i);
    for (auto comp : mComponentsRegistered) {
      comp.second->ActivateComponent(i);
    }
  }
  mInActiveEntities.clear();
}

void EntityManager::DeactivateAll() {
  for (auto i : mActiveEntities) {
    mInActiveEntities.insert(i);
    for (auto comp : mComponentsRegistered) {
      comp.second->DeactivateComponent(i);
    }
  }
  mActiveEntities.clear();
}

void EntityManager::ActivateAllExcept(const std::vector<EID> &entities) {
  if (entities.empty()) {
    ActivateAll();
    return;
  }

  std::vector<EID> toActivate;
  for (auto i : mInActiveEntities) {
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
  for (auto i : mActiveEntities) {
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
  auto iActive = mInActiveEntities.find(id);
  if (iActive != mInActiveEntities.end()) {
    for (auto comp = mComponentsRegistered.begin();
         comp != mComponentsRegistered.end(); comp++) {
      comp->second->ActivateComponent(id);
      mActiveEntities.insert(id);
      mInActiveEntities.erase(id);
    }
  }
}
void EntityManager::Deactivate(EID id) {
  // Check if EID is active
  auto iActive = mActiveEntities.find(id);
  if (iActive != mActiveEntities.end()) {
    for (auto comp = mComponentsRegistered.begin();
         comp != mComponentsRegistered.end(); comp++) {
      comp->second->DeactivateComponent(id);
      mInActiveEntities.insert(id);
      mActiveEntities.erase(id);
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
      .addFunction("SetName", &EntityManager::MapNameToEID)
      .addFunction("NameLookup", &EntityManager::NameLookup)
      .endClass()
      .endNamespace();
}
