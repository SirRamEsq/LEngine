#include "CompCamera.h"
#include "../Kernel.h"
#include "math.h"

ComponentCamera::ComponentCamera(EID id, ComponentPosition* pos, const std::string& logFile) : BaseComponent(id, logFile){
    mPosition= pos;
}
ComponentCamera::~ComponentCamera(){

}

void ComponentCamera::Update(){
    Coord2df coordinates=mPosition->GetPositionWorld();
    mCamera.view.x=floor(coordinates.x+0.5f);
    mCamera.view.y=floor(coordinates.y+0.5f);
    mCamera.translation.x=coordinates.x;
    mCamera.translation.y=coordinates.y;
}
void ComponentCamera::HandleEvent(const Event* event){

}

void ComponentCamera::SetAsMainCamera(){
    //mCamera.SetAsMainCamera();
}

const CRect& ComponentCamera::GetViewport(){
    return mCamera.view;
}



void ComponentCameraManager::Update(){
    compMapIt i=componentList.begin();
    for(; i!=componentList.end(); i++){
        i->second->Update();
    }
}

ComponentCameraManager::ComponentCameraManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_CAMERA", e){
}

ComponentCameraManager::~ComponentCameraManager(){
}

void ComponentCameraManager::AddComponent(EID id){
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentCamera* cam=new ComponentCamera(id, (ComponentPosition*)Kernel::stateMan.GetCurrentState()->comPosMan.GetComponent(id), logFileName);
    componentList[id]=cam;
}

