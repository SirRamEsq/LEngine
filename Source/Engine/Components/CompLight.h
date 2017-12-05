#ifndef L_ENGINE_COMP_LIGHT
#define L_ENGINE_COMP_LIGHT

#include <memory>

#include "../BaseComponent.h"
#include "../BaseComponentManager.h"
#include "../RenderManager.h"
#include "../Resources/RSC_GLShader.h"
#include "../Resources/RSC_Texture.h"
#include "../glslHelper.h"

#include "lights/Light.h"
#include "CompPosition.h"

#define MAX_LIGHTS 64

// forward declare
class ComponentLightManager;

class ComponentLight : public BaseComponent {
  friend class ComponentLightManager;

 public:
  ComponentLight(EID id, ComponentPosition *pos,
                 ComponentLightManager *manager);
  ~ComponentLight();

  /// Returns true if light with id exists
  bool LightExists(int id);

  /// Adds a light, takes ownership of the light, returns an index referring to
  /// the light
  int AddLight(std::unique_ptr<Light> light);

  void EnableLight(int index);
  void DisableLight(int index);

  void Update();

 protected:
  std::map<int, std::unique_ptr<Light>> *GetLights();
  Light *GetLight(int index);
  ComponentPosition *myPos;
  int mLastLightID;
  std::map<int, std::unique_ptr<Light>> mLights;
};

class ComponentLightManager : public BaseComponentManager_Impl<ComponentLight> {
 public:
  ComponentLightManager(EventDispatcher *e);

  std::unique_ptr<ComponentLight> ConstructComponent(EID id,
                                                     ComponentLight *parent);
  void Update();
  void Render(RSC_Texture *textureDiffuse, RSC_Texture *textureDestination);

  void BuildVAO();

  void SetAmbientLight(Vec4 light);

 private:
  GLuint FBO;  // frame buffer object id
  Vec4 mAmbientLight;
  const RSC_Texture *lightTexture;
  VAOWrapper vao;

  unsigned int numberOfLights;
};

#endif  // L_ENGINE_COMP_LIGHT
