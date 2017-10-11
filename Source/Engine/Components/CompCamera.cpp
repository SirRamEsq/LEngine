#include "CompCamera.h"
#include "../Kernel.h"
#include "math.h"

///////////////////
//ComponentCamera//
///////////////////
ComponentCamera::ComponentCamera(EID id, ComponentPosition* pos, RenderManager* rm, ComponentCameraManager* manager)
	: BaseComponent(id, manager), mCamera(rm, CRect(0,0, CAMERA_W, CAMERA_H)){
    mPosition= pos;
}
ComponentCamera::~ComponentCamera(){

}

void ComponentCamera::Update(){
    Coord2df coordinates=mPosition->GetPositionWorld();
    mCamera.view.x=floor(coordinates.x+0.5f);
    mCamera.view.y=floor(coordinates.y+0.5f);
}

void ComponentCamera::HandleEvent(const Event* event){

}

void ComponentCamera::SetViewport(CRect viewport){
	//mCamera.SetView(viewport);	
}

CRect ComponentCamera::GetViewport(){
    return mCamera.view;
}


//////////////////////////
//ComponentCameraManager//
//////////////////////////
ComponentCameraManager::ComponentCameraManager(EventDispatcher* e) : BaseComponentManager_Impl(e){
}

ComponentCameraManager::~ComponentCameraManager(){
}

std::unique_ptr<ComponentCamera> ComponentCameraManager::ConstructComponent	(EID id, ComponentCamera* parent){
    auto cam = std::make_unique<ComponentCamera>(id, (ComponentPosition*)dependencyPosition->GetComponent(id), dependencyRenderManager, this);
	return std::move(cam);
}

void ComponentCameraManager::SetDependencies(ComponentPositionManager* pos, RenderManager* rm){
	dependencyRenderManager = rm;
	dependencyPosition = pos;
}
