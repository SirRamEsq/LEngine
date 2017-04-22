#ifndef L_ENGINE_COMP_LIGHT
#define L_ENGINE_COMP_LIGHT

#include <memory>

#include "../Resources/LglShader.h"
#include "../glslHelper.h"
#include "../BaseComponent.h"
#include "../Resources/LTexture.h"
#include "../RenderManager.h"
#include "CompPosition.h"


#define MAX_LIGHTS 256

//forward declare
class ComponentLightManager;

class LightSource{
    public:
        LightSource(const float& iStart, const float& iEnd, const float& rad, const float& n, const Vec2& off);
        //How far the light reaches
        float radius;

        //Value between 0.0f and 1.0f
        float intensityStart;
        float intensityEnd;

        //Light flickering value
        float noise;

        //Whether or not light is rendered
        bool render;

        //Offset from center position of entity
        Vec2 offset;
};

class ComponentLight : public BaseComponent{
    friend class ComponentLightManager;

    public:
         ComponentLight (EID id, ComponentPosition* pos, const std::string& logFile);
        ~ComponentLight ();

        bool LightExists(const int& id);

        void Update     ();
        void HandleEvent(const Event* event);

        int  NewLightSource     (const float& intensityStart, const float& intensityEnd, const float& rad, const float& noise=0.5, const Vec2& offset=Vec2(0,0));

    protected:
        ComponentPosition* myPos;
        int numberOfLoadedLights;
        std::vector<LightSource> lightSources;
};

class ComponentLightManager : public BaseComponentManager{
    public:
        ComponentLightManager();

        void AddComponent(EID id);
        void HandleEvent(const Event* event);
        void Update();

        void Render(LTexture* textureDiffuse, LTexture* textureDestination, const Coord2d& topLeftCorner, L_GL_Program* shaderProgram);

        void BuildVAO();

        //Is only called once, used to set data that won't change (in this case, texture coordinates)
        void BuildVAOFirst();

    private:
        GLuint   FBO; //frame buffer object id
        const LTexture* lightTexture;
        VAOWrapper vao;

        unsigned int numberOfLights;
};

#endif // L_ENGINE_COMP_LIGHT

