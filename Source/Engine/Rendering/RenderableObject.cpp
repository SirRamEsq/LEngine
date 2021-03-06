#include "RenderableObject.h"
#include "../RenderManager.h"

std::map<RenderableObject::TYPE, std::string> RenderableObject::TYPE_STR = {
    {RenderableObject::TYPE::Generic, "Generic"},
    {RenderableObject::TYPE::Screen, "Screen"},
    {RenderableObject::TYPE::World, "World"},
    {RenderableObject::TYPE::SpriteBatch, "SpriteBatch"},
    {RenderableObject::TYPE::TileLayer, "TileLayer"},
    {RenderableObject::TYPE::Image, "Image"}};
////////////////////
// RenderableObject//
////////////////////
RenderableObject::RenderableObject(RenderManager *rm, TYPE t) : type(t) {
  depth = 0;
  scaleX = 1.0f;
  scaleY = 1.0f;
  rotation = 0.0f;
  render = true;
  shaderProgram = NULL;
  renderManager = rm;
}

bool RenderableObject::isTransparent(){
	//assume it is transparent
	return true;
}

void RenderableObject::SetDepth(const int &i) {
  depth = i;
}

int RenderableObject::GetDepth() { return depth; }

void RenderableObject::Render(const RenderCamera *camera) {
  Render(camera, shaderProgram);
}

void RenderableObject::Render() { Render(NULL, shaderProgram); }

std::string RenderableObject::GetTypeString() {
	return TYPE_STR[type];
}

//////////////////////////
// RenderableObjectScreen//
//////////////////////////
RenderableObjectScreen::RenderableObjectScreen(RenderManager *rm, TYPE t)
    : RenderableObject(rm, t) {}

void RenderableObjectScreen::AddToRenderManager() {
  renderManager->AddObjectScreen(this);
}

RenderableObjectScreen::~RenderableObjectScreen() {
  renderManager->RemoveObjectScreen(this);
}

/////////////////////////
// RenderableObjectWorld//
/////////////////////////
RenderableObjectWorld::RenderableObjectWorld(RenderManager *rm, TYPE t)
    : RenderableObject(rm, t) {}

void RenderableObjectWorld::AddToRenderManager() {
  renderManager->AddObjectWorld(this);
}

RenderableObjectWorld::~RenderableObjectWorld() {
  renderManager->RemoveObjectWorld(this);
}
