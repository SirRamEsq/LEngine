#include "../../../Source/Components/CompLight.h"
#include "../../../Source/Kernel.h"

//Only use this file if mocking
#ifdef TEST_ComponentLight_MOCK

///////////////
//LightSource//
///////////////
LightSource::LightSource(const float& iStart, const float& iEnd, const float& rad, const float& n, const Vec2& off)
    : intensityStart(iStart), intensityEnd(iEnd), radius(rad), noise(n), offset(off){
    render=true;
    radius=150;
}

//////////////////
//ComponentLight//
//////////////////

ComponentLight::ComponentLight (EID id, Coord2df pos, const std::string& logName) : BaseComponent(id, logName){
    myPos = pos;
    numberOfLoadedLights=0;

    //just for testing sake;
    NewLightSource(1.0f, 0.1f, 5.0f);
}
ComponentLight::~ComponentLight (){}

bool ComponentLight::LightExists(const int& id){
    return id<numberOfLoadedLights;
}

void ComponentLight::Update(){}

void ComponentLight::HandleEvent(const Event* event){

}

int ComponentLight::NewLightSource (const float& intensityStart, const float& intensityEnd, const float& rad, const float& noise, const Vec2& offset){
    LightSource light(intensityStart, intensityEnd, rad, noise, offset);
    lightSources.push_back(light);

    int returnValue=numberOfLoadedLights;
    numberOfLoadedLights++;

    return returnValue;
}

//////////////////////////
//ComponentLightManager//
//////////////////////////

ComponentLightManager::ComponentLightManager() : BaseComponentManager("LOG_COMP_LIGHT") {
}

void ComponentLightManager::AddComponent(EID id){
}

void ComponentLightManager::HandleEvent(const Event* event){

}

void ComponentLightManager::Update(){
}

void ComponentLightManager::Render(LTexture* textureDiffuse, LTexture* textureDestination, const Coord2d& topLeftCorner, L_GL_Program* shaderProgram){

}

void ComponentLightManager::BuildVAOFirst(){
}
void ComponentLightManager::BuildVAO(){

}

#endif // TEST_ComponentCamera_MOCK
