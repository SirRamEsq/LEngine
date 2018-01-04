#ifndef L_RENDER_MANAGER
#define L_RENDER_MANAGER

#include "Defines.h"
#include "SDLInit.h"
#include "glslHelper.h"
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "Rendering/RenderImageLayer.h"
#include "Rendering/RenderPrimitives.h"
#include "Rendering/RenderableSpriteBatch.h"
#include "Rendering/RenderTileLayer.h"
#include "Rendering/RenderableObject.h"
#include "Resources/RSC_GLShader.h"

#include "gui/imgui.h"

/*
Centralized Rendering:
The rendermanger will know of all renderable objects (sprites, backgrounds,
tiles, and text)
it will render them in order based on their depth
it will also control the camera
RenderableSprite automatically adds itself to the appropriate sprite batch when
instantiated.

RenderManager owns only SpriteBatches, the rest are owned by whatever
instantiates them
*/

class ComponentCamera;
class ComponentLightManager;
class RenderCamera {
  friend class RenderManager;
  friend class ComponentCamera;

 public:
  RenderCamera(RenderManager *rm, Rect viewPort);
  ~RenderCamera();

  void SetScaling(float s);
  void SetRotation(float r);
  float GetScaling() const;
  float GetRotation() const;

  void Bind(const GLuint &GlobalCameraUBO);

  Rect GetView() const;
  void SetView(Rect viewPort);
  Rect GetScreenSpace() const;
  void SetScreenSpace(Rect screen);

  void RenderFrameBufferTextureFinal(ComponentLightManager *lightMan,
                                     RSC_GLProgram *program);
  void RenderFrameBufferTextureDiffuse();

 protected:
  /// Resolution to display
  Rect view;

  RSC_Texture *GetFrameBufferTextureDiffuse() {
    return frameBufferTextureDiffuse.get();
  }
  RSC_Texture *GetFrameBufferTextureFinal() {
    return frameBufferTextureFinal.get();
  }

 private:
  void RenderFrameBufferTexture(const RSC_Texture *tex);
  float rotation;
  float scale;

  MAP_DEPTH nearClippingPlane;
  MAP_DEPTH farClippingPlane;

  /// Coordinates on the screen that the image will be rendered to
  /// All coordinates normalized between 0.0 and 1.0
  Rect screenSpace;

  /// frame buffer object id
  GLuint FBO;
  // Depth tex id
  GLuint mDepthTextureID;

  bool mActive;

  std::unique_ptr<RSC_Texture> frameBufferTextureDiffuse;
  std::unique_ptr<RSC_Texture> frameBufferTextureFinal;

  RenderManager *dependencyRenderManager;
};

class RenderManager {
  friend RenderableObjectScreen;
  friend RenderableObjectWorld;
  friend RenderCamera;

 public:
  RenderManager();

  void Render();
  unsigned int GetTimeElapsed();

  void AssignCameraUBO(RSC_GLProgram *program);

  // returns sprite batch that supports 'textureName' and has room for at least
  // numSprites more room
  RenderableSpriteBatch *GetSpriteBatch(const RSC_Texture *tex,
                                    const RSC_Texture *texNormal,
                                    const MAP_DEPTH &depth,
                                    const int &numSprites);
  MAP_DEPTH nextTextID;

  std::unique_ptr<RSC_GLProgram> LoadShaderProgram(
      const std::string &vertexName, const std::string &fragName);
  void SetupUniformBuffers(RSC_GLProgram *program);

 protected:
  /**
   * Will add camera to list of cameras to be rendered
   * is meant to be called by a newly created camera
   */
  bool AddCamera(RenderCamera *cam);
  /**
   * Will remove camera from list of cameras to be rendered
   * is meant to be called by a camera destructor
   */
  void RemoveCamera(RenderCamera *cam);

  /**
   * Will add object to list of object to be rendered
   * is meant to be called by a newly created object to screen coordinates
   */
  void AddObjectScreen(RenderableObjectScreen *obj);
  void RemoveObjectScreen(RenderableObjectScreen *obj);

  /**
   * Will add object to list of object to be rendered
   * is meant to be called by a newly created object to World coordinates
   */
  void AddObjectWorld(RenderableObjectWorld *obj);
  void RemoveObjectWorld(RenderableObjectWorld *obj);

 private:
  /**
   * Will Load default shaders for Tile, Light Sprite, Image if not already
   * loaded
   * will assign default shaders to member data
   */
  void LoadDefaultShaders();

  void ProcessDrawCall(RenderableObject *obj, RenderCamera *camera);

  void ImGuiRender(ImDrawData *drawData);

  bool listChange;
  /// \TODO decide if these should be lists or sets
  std::list<RenderableObjectWorld *> objectsWorld;
  std::list<RenderableObjectScreen *> objectsScreen;
  std::set<RenderCamera *> mCameras;

  static const std::string defaultProgramTileName;
  static const std::string defaultProgramSpriteName;
  static const std::string defaultProgramLightName;
  static const std::string defaultProgramImageName;

  RSC_GLProgram defaultProgramTile;
  RSC_GLProgram defaultProgramSprite;
  RSC_GLProgram defaultProgramLight;
  RSC_GLProgram defaultProgramImage;

  static GLuint GlobalCameraUBO;
  static const GLuint CameraDataBindingIndex;

  static GLuint GlobalProgramUBO;
  static const GLuint ProgramDataBindingIndex;

  int spriteBatchMaxSize;
  std::map<MAP_DEPTH,                              // depth
           std::map<const RSC_Texture *,           // texture
                    std::map<const RSC_Texture *,  // textureNormal
                             std::vector<std::unique_ptr<RenderableSpriteBatch>>>>>
      spriteBatchMap;

  // Time since creation of RenderManager
  unsigned int timeElapsed = 0;
};

#endif
