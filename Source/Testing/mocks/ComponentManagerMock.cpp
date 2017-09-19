#include "ComponentManagerMock.h"

ComponentTest::ComponentTest(EID id, const std::string& logName) : BaseComponent(id, logName){
	updateCount = 0;
}

ComponentTest::~ComponentTest(){

}

void ComponentTest::Update(){
	updateCount++;
}

unsigned int ComponentTest::GetUpdateCount(){
	return updateCount;
}

void ComponentTest::HandleEvent(const Event* event){

}

////////////////////////
//ComponentTestManager//
////////////////////////
ComponentTestManager::ComponentTestManager(EventDispatcher* e) : BaseComponentManager("LOGTEST", e){

}

void ComponentTestManager::AddComponent(EID id, EID parent){
    auto i = componentList.find(id);
    if(i!=componentList.end()){return;}
    auto sprite = make_unique<ComponentTest>(id, logFileName);
    componentList[id] = std::move(sprite);
}

