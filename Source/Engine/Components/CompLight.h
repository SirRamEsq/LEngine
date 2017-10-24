#ifndef L_ENGINE_COMP_LIGHT
#define L_ENGINE_COMP_LIGHT

#include <memory>

#include "../BaseComponent.h"
#include "../BaseComponentManager.h"
#include "../RenderManager.h"
#include "../Resources/RSC_GLShader.h"
#include "../Resources/RSC_Texture.h"
#include "../glslHelper.h"
#include "CompPosition.h"

#define MAX_LIGHTS 256

// forward declare
class ComponentLightManager;

class LightSource {
 public:
  LightSource(const float &iStart, const float &iEnd, const float &rad,
              const float &n, const Vec2 &off);
  // How far the light reaches
  float radius;

  // Value between 0.0f and 1.0f
  float intensityStart;
  float intensityEnd;

  // Light flickering value
  float noise;

  // Whether or not light is rendered
  bool render;

  // Offset from center position of entity
  Vec2 offset;
};

class ComponentLight : public BaseComponent {
  friend class ComponentLightManager;

 public:
  ComponentLight(EID id, ComponentPosition *pos,
                 ComponentLightManager *manager);
  ~ComponentLight();

  bool LightExists(const int &id);

  void Update();

  int NewLightSource(const float &intensityStart, const float &intensityEnd,
                     const float &rad, const float &noise = 0.5,
                     const Vec2 &offset = Vec2(0, 0));

 protected:
  ComponentPosition *myPos;
  int numberOfLoadedLights;
  std::vector<LightSource> lightSources;
};

class ComponentLightManager : public BaseComponentManager_Impl<ComponentLight> {
 public:
  ComponentLightManager(EventDispatcher *e);

  std::unique_ptr<ComponentLight> ConstructComponent(EID id,
                                                     ComponentLight *parent);
  void Update();

  void Render(RSC_Texture *textureDiffuse, RSC_Texture *textureDestination,
              const RSC_GLProgram *shaderProgram);

  void BuildVAO();

  // Is only called once, used to set data that won't change (in this case,
  // texture coordinates)
  void BuildVAOFirst();

 private:
  GLuint FBO;  // frame buffer object id
  const RSC_Texture *lightTexture;
  VAOWrapper vao;

  unsigned int numberOfLights;
};

#endif  // L_ENGINE_COMP_LIGHT
