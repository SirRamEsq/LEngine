#include "RenderableObject.h"
#include "../Kernel.h"

////////////////////
//RenderableObject//
////////////////////
RenderableObject::RenderableObject(TYPE t) : type(t){
    depth=0;
    scaleX=1.0f;
    scaleY=1.0f;
    rotation=0.0f;
    render=true;
    shaderProgram=NULL;
}

void RenderableObject::SetDepth(const int& i){
    depth=i;
    Kernel::stateMan.GetCurrentState()->renderMan.OrderOBJs();
}

int RenderableObject::GetDepth(){return depth;}

//////////////////////////
//RenderableObjectScreen//
//////////////////////////
RenderableObjectScreen::RenderableObjectScreen(TYPE t) :  RenderableObject(t){

}

void RenderableObjectScreen::AddToRenderManager(){
    Kernel::stateMan.GetCurrentState()->renderMan.AddObjectScreen(this);
}

RenderableObjectScreen::~RenderableObjectScreen(){
    Kernel::stateMan.GetCurrentState()->renderMan.RemoveObjectScreen(this);
}

/////////////////////////
//RenderableObjectWorld//
/////////////////////////
RenderableObjectWorld::RenderableObjectWorld(TYPE t) : RenderableObject(t){
}

void RenderableObjectWorld::AddToRenderManager(){
    Kernel::stateMan.GetCurrentState()->renderMan.AddObjectWorld(this);
}

RenderableObjectWorld::~RenderableObjectWorld(){
    Kernel::stateMan.GetCurrentState()->renderMan.RemoveObjectWorld(this);
}
