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

int ComponentLight::AddLight(std::unique_ptr<Light> light) {
  int thisLightID = mLastLightID;
  mLastLightID++;
  mLights[thisLightID] = std::move(light);
  return thisLightID;
}
PointLight *ComponentLight::CreatePointLight() {
  int thisLightID = mLastLightID;
  mLastLightID++;
  auto newLight = std::make_unique<PointLight>();
  auto newLightRaw = newLight.get();
  mLights[thisLightID] = std::move(newLight);
  return newLightRaw;
}

void ComponentLight::Update() {
  auto worldPos = myPos->GetPositionWorld();
  for (auto light = mLights.begin(); light != mLights.end(); light++) {
    light->second->worldPos.x = worldPos.x + light->second->pos.x;
    light->second->worldPos.y = worldPos.y + light->second->pos.y;
  }
}

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
    : BaseComponentManager_Impl(e), vao((VAO_TEXTURE), 1) {
  if (GlobalLightUBO == 0) {
    CreateLightUBO();
  }
  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  mAmbientLight.color.x = 1.0;
  mAmbientLight.color.y = 1.0;
  mAmbientLight.color.z = 1.0;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  BuildVAO();
}

ComponentLightManager::~ComponentLightManager() {
  glDeleteFramebuffers(1, &FBO);
}

void ComponentLightManager::Update() {
  AddNewComponents();
  // Update all entities
  for (auto i = mComponentList.begin(); i != mComponentList.end(); i++) {
    UpdateComponent(i->second.get());
  }

  // Reset all 'updatedThisFrame' bits
  for (auto i = mComponentList.begin(); i != mComponentList.end(); i++) {
    i->second->updatedThisFrame = false;
  }

  std::vector<Light *> lights;
  for (auto comp = mActiveComponents.begin(); comp != mActiveComponents.end();
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
  UpdateLightUBO(lights);
}

void ComponentLightManager::CreateLightUBO() {
  // MAX_LIGHTS set of 3 vec4 plus an integer
  GLuint bufferSize = sizeof(int) + (MAX_LIGHTS * (sizeof(float) * 4) * 3);
  glGenBuffers(1, &GlobalLightUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, GlobalLightUBO);
  glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
  // Bind generated CPU buffer to the index
  glBindBufferBase(GL_UNIFORM_BUFFER, LightBindingIndex, GlobalLightUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ComponentLightManager::BindLightUBO(RSC_GLProgram *program) {
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
    pos[0] = light->worldPos.x;
    pos[1] = light->worldPos.y;
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
  glBufferSubData(GL_UNIFORM_BUFFER, lightCountOffset, sizeof(int),
                  &totalLights);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
#ifndef DEBUG_MODE
  auto GLerror = GL_GetError();
  if (GLerror != "") {
    LOG_DEBUG(GLerror);
  }
#endif
}

void ComponentLightManager::Render(GLuint textureDiffuse, GLuint textureDepth,
                                   GLuint textureDestination,
                                   unsigned int width, unsigned int height,
                                   RSC_GLProgram *program) {
  // Setup framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureDestination, 0);

  // Bind textures
  RSC_Texture::SetActiveTexture(GL_TEXTURE0);
  RSC_Texture::Bind(textureDiffuse);
  RSC_Texture::SetActiveTexture(GL_TEXTURE1);
  RSC_Texture::Bind(textureDepth);

  vao.Bind();

  program->Bind();
  // Then bind the uniform samplers to texture units:
  glUniform1i(program->GetUniformLocation("diffuseTex"), 0);
  glUniform1i(program->GetUniformLocation("depthTex"), 1);

  float ambientLight[3];
  ambientLight[0] = mAmbientLight.color.x;
  ambientLight[1] = mAmbientLight.color.y;
  ambientLight[2] = mAmbientLight.color.z;
  glUniform3fv(program->GetUniformLocation("AMBIENT_COLOR"), 1,
               &ambientLight[0]);

  auto fbError = GL_CheckFramebuffer();
  if (fbError != "") {
    LOG_ERROR(fbError);
  }

  glDrawArrays(GL_QUADS, 0, 4);

  // Unbind everything
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(NULL);

  // Unbind Textures
  RSC_Texture::BindNull();
  RSC_Texture::SetActiveTexture(GL_TEXTURE0);
  RSC_Texture::BindNull();

  RSC_GLProgram::BindNULL();
  auto GLerror = GL_GetError();
  if (GLerror != "") {
    LOG_DEBUG(GLerror);
  }
}

void ComponentLightManager::BuildVAO() {
  float Left = 0;
  float Right = 1;
  float Top = 0;
  float Bottom = 1;
  vao.GetTextureArray()[0].x = Left;
  vao.GetTextureArray()[0].y = Top;

  vao.GetTextureArray()[1].x = Right;
  vao.GetTextureArray()[1].y = Top;

  vao.GetTextureArray()[2].x = Right;
  vao.GetTextureArray()[2].y = Bottom;

  vao.GetTextureArray()[3].x = Left;
  vao.GetTextureArray()[3].y = Bottom;

  vao.GetVertexArray()[0].x = -1.0;
  vao.GetVertexArray()[0].y = -1.0;

  vao.GetVertexArray()[1].x = 1.0;
  vao.GetVertexArray()[1].y = -1.0;

  vao.GetVertexArray()[2].x = 1.0;
  vao.GetVertexArray()[2].y = 1.0;

  vao.GetVertexArray()[3].x = -1.0;
  vao.GetVertexArray()[3].y = 1.0;

  vao.UpdateGPU();
}

std::unique_ptr<ComponentLight> ComponentLightManager::ConstructComponent(
    EID id, ComponentLight *parent) {
  auto dependencyPosition = &Kernel::stateMan.GetCurrentState()->comPosMan;
  if (dependencyPosition->GetComponent(id) == NULL) {
    dependencyPosition->AddComponent(id);
  }
  auto light = std::make_unique<ComponentLight>(
      id, (ComponentPosition *)dependencyPosition->GetComponent(id), this);

  return std::move(light);
}

void ComponentLightManager::SetAmbientLight(Vec3 color) {
  mAmbientLight.color = color;
}
Vec3 ComponentLightManager::GetAmbientLight() { return mAmbientLight.color; }

void ComponentLightManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")

      .beginClass<ComponentLightManager>("ComponentLightManager")
      .addFunction("GetAmbient", &ComponentLightManager::GetAmbientLight)
      .addFunction("SetAmbient", &ComponentLightManager::SetAmbientLight)
      .endClass()

      .deriveClass<ComponentLight, BaseComponent>("ComponentLight")
      .addFunction("CreatePointLight", &ComponentLight::CreatePointLight)
      .endClass()

      .beginClass<Light>("Light")
      .addData("color", &Light::color)
      .addData("pos", &Light::pos)
      .addData("noise", &Light::noise)
      .addData("distance", &Light::distance)
      .addData("render", &Light::render)
      .endClass()

      .deriveClass<PointLight, Light>("PointLight")
      .endClass()

      .endNamespace();
}
