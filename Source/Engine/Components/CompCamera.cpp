#include "CompCamera.h"
#include "../Kernel.h"
#include "math.h"

///////////////////
//ComponentCamera//
///////////////////
ComponentCamera::ComponentCamera(EID id, ComponentPosition* pos, RenderManager* rm, const std::string& logFile) : BaseComponent(id, logFile), mCamera(rm){
    mPosition= pos;
}
ComponentCamera::~ComponentCamera(){

}

void ComponentCamera::Update(){
    Coord2df coordinates=mPosition->GetPositionWorld();
    mCamera.view.x=floor(coordinates.x+0.5f);
    mCamera.view.y=floor(coordinates.y+0.5f);
    //mCamera.translation.x=coordinates.x;
    //mCamera.translation.y=coordinates.y;
}
void ComponentCamera::HandleEvent(const Event* event){

}


const CRect& ComponentCamera::GetViewport(){
    return mCamera.view;
}


//////////////////////////
//ComponentCameraManager//
//////////////////////////
ComponentCameraManager::ComponentCameraManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_CAMERA", e){
}

ComponentCameraManager::~ComponentCameraManager(){
}

void ComponentCameraManager::AddComponent(EID id, EID parent){
    auto i=componentList.find(id);
    if(i!=componentList.end()){return;}
    auto cam = make_unique<ComponentCamera>(id, (ComponentPosition*)dependencyPosition->GetComponent(id), dependencyRenderManager, logFileName);
    componentList[id] = std::move(cam);
}

void ComponentCameraManager::SetDependencies(ComponentPositionManager* pos, RenderManager* rm){
	dependencyRenderManager = rm;
	dependencyPosition = pos;
}
