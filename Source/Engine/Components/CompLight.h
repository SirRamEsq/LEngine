#ifndef L_ENGINE_COMP_LIGHT
#define L_ENGINE_COMP_LIGHT

#include <memory>

#include "../BaseComponent.h"
#include "../BaseComponentManager.h"
#include "../RenderManager.h"
#include "../Resources/RSC_GLShader.h"
#include "../Resources/RSC_Texture.h"
#include "../Vector.h"

#include "../Rendering/VAOWrapper.h"

#include "lights/Light.h"
#include "lights/PointLight.h"
#include "CompPosition.h"

#define MAX_LIGHTS 32

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
  PointLight* CreatePointLight();

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
  ~ComponentLightManager();

  std::unique_ptr<ComponentLight> ConstructComponent(EID id,
                                                     ComponentLight *parent);
  void Update();
  /// \TODO pass textureNormal as optional param
  void Render(GLuint textureDiffuse, GLuint textureDepth,
              GLuint textureDestination, unsigned int width,
              unsigned int height, RSC_GLProgram *program);

  void BuildVAO();

  void SetAmbientLight(Vec3 color);

  static void BindLightUBO(RSC_GLProgram *program);

 private:
  static void CreateLightUBO();
  static void UpdateLightUBO(std::vector<Light *> lights);
  GLuint FBO;  // frame buffer object id
  VAOWrapper2D vao;

  Light mAmbientLight;

  static GLuint GlobalLightUBO;
  static const GLuint LightBindingIndex;
};

#endif  // L_ENGINE_COMP_LIGHT
