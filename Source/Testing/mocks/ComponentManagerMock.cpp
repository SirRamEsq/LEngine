#include "ComponentManagerMock.h"

ComponentTest::ComponentTest(EID id, ComponentTestManager *manager)
    : BaseComponent(id, manager) {
  updateCount = 0;
}

ComponentTest::~ComponentTest() {}

void ComponentTest::Update() { updateCount++; }

unsigned int ComponentTest::GetUpdateCount() { return updateCount; }

void ComponentTest::HandleEvent(const Event *event) {}

////////////////////////
// ComponentTestManager//
////////////////////////
ComponentTestManager::ComponentTestManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e) {}

std::unique_ptr<ComponentTest> ComponentTestManager::ConstructComponent(
    EID id, ComponentTest *parent) {
  auto component = std::make_unique<ComponentTest>(id, this);
  component->SetParent(parent);
  return std::move(component);
}
