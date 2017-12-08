#include "CompLight.h"
#include "../Kernel.h"

#include "../Resolution.h"

bool SortLights(Light *l1, Light *l2) {
  if (l1->type > l2->type) {
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
GLuint ComponentLightManager::GlobalLightUBO = 0;
const GLuint ComponentLightManager::LightBindingIndex = 3;

ComponentLightManager::ComponentLightManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e), vao(0, MAX_LIGHTS) {
  glGenFramebuffers(1, &FBO);
  mAmbientLight.color.x = 1.0f;
  mAmbientLight.color.y = 1.0f;
  mAmbientLight.color.z = 1.0f;
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
  //UpdateLightUBO(lights);
}

void ComponentLightManager::CreateLightUBO() {
  GLuint bufferSize = (sizeof(float) * 16) * 3;
  glGenBuffers(1, &GlobalLightUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, GlobalLightUBO);
  glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
  // Bind generated CPU buffer to the index
  glBindBufferBase(GL_UNIFORM_BUFFER, LightBindingIndex, GlobalLightUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ComponentLightManager::BindLightUBO(RSC_GLProgram *program) {
  if (GlobalLightUBO == 0) {
    CreateLightUBO();
  }
  try {
    GLuint programHandle = program->GetHandle();
    GLuint uboHandle = program->GetUniformBlockHandle("LightData");
    // Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, uboHandle, LightBindingIndex);
  } catch (LEngineShaderProgramException e) {
  }
}

void ComponentLightManager::UpdateLightUBO(std::vector<Light *> lights) {
  glBindBuffer(GL_UNIFORM_BUFFER, GlobalLightUBO);

  int offset = 0;
  // all elements are 4 floats
  int dataSize = (sizeof(float) * 4);
  for (auto i = lights.begin(); i != lights.end(); i++) {
    auto light = *i;
    float pos[4];
    pos[0] = light->pos.x;
    pos[1] = light->pos.y;
    pos[2] = light->pos.z;
    pos[3] = 1.0;
    float color[4];
    color[0] = light->color.x;
    color[1] = light->color.y;
    color[2] = light->color.z;
    color[3] = 1.0;
    float extra[4];
    extra[0] = light->distance;
    extra[1] = light->noise;
    extra[2] = 0;
    extra[3] = 0;

    glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &pos[0]);
    offset += dataSize;

    glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &color[0]);
    offset += dataSize;

    glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &extra[0]);
    offset += dataSize;
  }

  // offset of variable location for total light count
  int lightCountOffset = MAX_LIGHTS * (dataSize * 3);
  int totalLights = lights.size();
  glBufferSubData(GL_UNIFORM_BUFFER, lightCountOffset, sizeof(int), &totalLights);
}

void ComponentLightManager::Render(GLuint textureDiffuse, GLuint textureDepth,
                                   GLuint textureDestination,
                                   unsigned int width, unsigned int height,
                                   RSC_GLProgram *program) {
  program->Bind();

  // Setup framebuffer
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
  RSC_Texture::Bind(textureDiffuse);
  glActiveTexture(GL_TEXTURE1);
  RSC_Texture::Bind(textureDepth);

  // Set Render Viewport
  glViewport(0, 0, width, height);

  // Atatch buffer texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureDestination, 0);

  // glBindVertexArray(vao.GetVAOID());
  // glDrawArrays(GL_QUADS, 0, numberOfLights * 4);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, width, height, 0, 0, 1);  // 2D
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Unbind everything
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(NULL);

  // Back to initial viewport
  glPopAttrib();

  // Unbind Textures
  RSC_Texture::BindNull();
  glActiveTexture(GL_TEXTURE0);
  RSC_Texture::BindNull();

  RSC_GLProgram::BindNULL();
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
