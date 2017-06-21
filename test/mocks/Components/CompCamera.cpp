#include "CompCamera.h"
#include "math.h"
//Only use this file if mocking
#ifdef TEST_ComponentCamera_MOCK

ComponentCamera::ComponentCamera(EID id, const std::string& logFile)
: BaseComponent(id, logFile), mCamera(0.0f, 0.0f, 16.0f, 16.0f){
}
ComponentCamera::~ComponentCamera(){

}

void ComponentCamera::Update(){
    Coord2df coordinates(1.0f, 1.0f);//mPosition->GetPositionWorld();
}
void ComponentCamera::HandleEvent(const Event* event){

}

void ComponentCamera::SetAsMainCamera(){
    //mCamera.SetAsMainCamera();
}

const CRect& ComponentCamera::GetViewport(){
    return mCamera;
}



void ComponentCameraManager::Update(){
    compMapIt i=componentList.begin();
    for(; i!=componentList.end(); i++){
        i->second->Update();
    }
}

ComponentCameraManager::ComponentCameraManager() : BaseComponentManager("LOG_COMP_CAMERA"){
}

ComponentCameraManager::~ComponentCameraManager(){
}

void ComponentCameraManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentCamera* cam=new ComponentCamera(id, logFileName);
    componentList[id]=cam;
}

#endif // TEST_ComponentCamera_MOCK
