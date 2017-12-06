#include "CompLight.h"
#include "../Kernel.h"

// Will sort lights based on shader program address
// this will group lights that share a program together
// Will this lead to weird and variable rendering orders? probably...
bool SortLights(Light *l1, Light *l2) {
  if (l1->shaderProgram > l2->shaderProgram) {
    return true;
  }
  return false;
}

//////////////////
// ComponentLight//
//////////////////

ComponentLight::ComponentLight(EID id, ComponentPosition *pos,
                               ComponentLightManager *manager)
    : BaseComponent(id, manager) {
  myPos = pos;
  mLastLightID = 0;
}
ComponentLight::~ComponentLight() {}

bool ComponentLight::LightExists(int id) {
  auto light = mLights.find(id);
  return (light != mLights.end());
}

void ComponentLight::Update() {}

void ComponentLight::EnableLight(int index) {
  auto light = GetLight(index);
  if (light == NULL) {
    return;
  }
  light->render = true;
}

void ComponentLight::DisableLight(int index) {
  auto light = GetLight(index);
  if (light == NULL) {
    return;
  }
  light->render = false;
}

Light *ComponentLight::GetLight(int index) {
  auto light = mLights.find(index);
  if (light != mLights.end()) {
    return light->second.get();
  }
  return NULL;
}

std::map<int, std::unique_ptr<Light>> *ComponentLight::GetLights() {
  return &mLights;
}

//////////////////////////
// ComponentLightManager//
//////////////////////////
ComponentLightManager::ComponentLightManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e), vao(0, MAX_LIGHTS) {
  glGenFramebuffers(1, &FBO);
  mAmbientLight.x = 1.0f;
  mAmbientLight.y = 1.0f;
  mAmbientLight.z = 1.0f;
  mAmbientLight.w = 0.0f;
}

void ComponentLightManager::Update() {
  // Update all entities
  for (auto i = componentList.begin(); i != componentList.end(); i++) {
    UpdateComponent(i->second.get());
  }

  // Reset all 'updatedThisFrame' bits
  for (auto i = componentList.begin(); i != componentList.end(); i++) {
    i->second->updatedThisFrame = false;
  }

  // BuildVAO();
}

void ComponentLightManager::Render(GLuint textureDiffuse, GLuint textureDepth,
                                   GLuint textureDestination,
                                   unsigned int width, unsigned int height) {
  std::vector<Light *> lights;
  for (auto comp = activeComponents.begin(); comp != activeComponents.end();
       comp++) {
    auto compLights = comp->second->GetLights();
    for (auto lightIt = compLights->begin(); lightIt != compLights->end();
         lightIt++) {
      auto light = lightIt->second.get();
      if (light->render) {
        lights.push_back(light);
      }
      if (lights.size() >= MAX_LIGHTS) {
        // breakout of both for loops
        goto breakout;
      }
    }
  }
// Look ma! A decent goto!
breakout:

  std::sort(lights.begin(), lights.end(), &SortLights);

  return;

  // Setup framebuffer
  RSC_GLProgram::BindNULL();
  RSC_Texture::BindNull();
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureDestination, 0);
  // Clear
  glClearColor(0.25f, 0.55f, 0.85f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, NULL);

  // Push viewport bit
  glPushAttrib(GL_VIEWPORT_BIT);
  // Setup frame buffer render
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  // Bind textures
  glActiveTexture(GL_TEXTURE1);
  RSC_Texture::Bind(textureDiffuse);
  glActiveTexture(GL_TEXTURE2);
  RSC_Texture::Bind(textureDepth);

  // Set Render Viewport
  glViewport(0, 0, width, height);

  // Atatch buffer texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureDestination, 0);

  glBindVertexArray(vao.GetVAOID());
  glDrawArrays(GL_QUADS, 0, numberOfLights * 4);

  // Unbind everything
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(NULL);
  RSC_GLProgram::BindNULL();

  // Back to initial viewport
  glPopAttrib();

  // Unbind Textures
  glActiveTexture(GL_TEXTURE2);
  RSC_Texture::BindNull();
  glActiveTexture(GL_TEXTURE1);
  RSC_Texture::BindNull();
  glActiveTexture(GL_TEXTURE0);
  RSC_Texture::BindNull();
}

void ComponentLightManager::BuildVAO() {
  /*
  unsigned int vertex = 0;
  unsigned int lightCount = 0;

  for (auto i = componentList.begin(); i != componentList.end(); i++) {
    auto lightSources = ((ComponentLight *)(i->second.get()))->lightSources;
    for (auto lightSource = lightSources.begin();
         lightSource != lightSources.end(); lightSource++) {
      float radius = lightSource->radius;
      auto pos =
          ((ComponentLight *)(i->second.get()))->myPos->GetPositionWorld();
      pos = pos.Round();

      // Will subtract camera translation in shader later on during rendering

      Vec2 topLeftVertex(-radius + pos.x, -radius + pos.y);
      Vec2 topRightVertex(radius + pos.x, -radius + pos.y);
      Vec2 bottomRightVertex(radius + pos.x, radius + pos.y);
      Vec2 bottomLeftVertex(-radius + pos.x, radius + pos.y);

      vao.GetVertexArray()[vertex] = topLeftVertex;
      vao.GetVertexArray()[vertex + 1] = topRightVertex;
      vao.GetVertexArray()[vertex + 2] = bottomRightVertex;
      vao.GetVertexArray()[vertex + 3] = bottomLeftVertex;

      lightCount += 1;
      vertex += 4;
    }
  }

  numberOfLights = lightCount;
  vao.UpdateGPU();
  */
}

std::unique_ptr<ComponentLight> ComponentLightManager::ConstructComponent(
    EID id, ComponentLight *parent) {
  auto light = std::make_unique<ComponentLight>(
      id, (ComponentPosition *)Kernel::stateMan.GetCurrentState()
              ->comPosMan.GetComponent(id),
      this);

  return std::move(light);
}
