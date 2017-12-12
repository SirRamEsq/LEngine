#include "RenderManager.h"

#include "Components/CompSprite.h"
#include "Kernel.h"
#include "Resolution.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "Components/CompLight.h"

#include "Resources/RSC_Map.h"

#include <sstream>

/*
 * Lighting Plan
 * Render opague geometry to Diffuse with depth buffer
 * Render Lights on diffuse to final texture
 * 	Lights the are behind a fragment do not affect the fragment
 * 	Lights that strike a fragment in the 'block range' will be blocked
 * 	Lights that are in front of a fragment affect the fragment
 *
 * Render transparent geometry to final texture
 */

////////////////
// RenderCamera//
////////////////
RenderCamera::RenderCamera(RenderManager *rm, Rect viewPort)
    : frameBufferTextureDiffuse(std::unique_ptr<RSC_Texture>(
          new RSC_Texture(viewPort.w, viewPort.h, 4, GL_RGBA))),
      frameBufferTextureFinal(std::unique_ptr<RSC_Texture>(
          new RSC_Texture(viewPort.w, viewPort.h, 4, GL_RGBA))),
      dependencyRenderManager(rm) {
  scale = 1;
  screenSpace = Rect(0, 0, 1.0, 1.0);
  rotation = 0;
  nearClippingPlane = -100;
  farClippingPlane = 100;
  frameBufferTextureDiffuse->GenerateID();
  frameBufferTextureFinal->GenerateID();
  SetView(viewPort);

  glGenFramebuffers(1, &FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  // Create depth texture
  glGenTextures(1, &mDepthTextureID);
  glBindTexture(GL_TEXTURE_2D, mDepthTextureID);

  // GL_LINEAR does not make sense for depth texture. However, next tutorial
  // shows usage of GL_LINEAR and PCF
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Remove artefact on the edges of the shadowmap
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewPort.w, viewPort.h,
               0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
  RSC_Texture::BindNull();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  dependencyRenderManager->AddCamera(this);
}

RenderCamera::~RenderCamera() {
  glDeleteFramebuffers(1, &FBO);
  glDeleteTextures(1, &mDepthTextureID);
  // should this be called? what if this fbo is currently bound?
  // glBindFramebuffer(GL_FRAMEBUFFER, 0);
  dependencyRenderManager->RemoveCamera(this);
}

void RenderCamera::SetScaling(float s) { scale = s; }

void RenderCamera::SetRotation(float r) { rotation = r; }

float RenderCamera::GetScaling() const { return scale; }

float RenderCamera::GetRotation() const { return rotation; }

Rect RenderCamera::GetView() const { return view; }

void RenderCamera::SetView(Rect viewPort) { view = viewPort; }

Rect RenderCamera::GetScreenSpace() const { return screenSpace; }

void RenderCamera::SetScreenSpace(Rect screen) { screenSpace = screen; }

void RenderCamera::Bind(const GLuint &GlobalCameraUBO) {
  Matrix4 T = Matrix4::IdentityMatrix();
  T = T.Translate(Vec3(-view.x, -view.y, 0.0));

  Matrix4 R = Matrix4::IdentityMatrix();
  R = R.RotateZ(0);

  Matrix4 S = Matrix4::IdentityMatrix();
  S = S.Scale(Vec3(1, 1, 1));

  // Matrix4 viewMat = T * R * S;
  Matrix4 viewMat = S * R * T;

  // Will render texture upside down
  Matrix4 projectionMat = Matrix4::OrthoGraphicProjectionMatrix(
      // Coord2df(view.w, view.h), nearClippingPlane, farClippingPlane);
      Coord2df(view.w, view.h), nearClippingPlane, farClippingPlane);
  auto projectionMatInverse = projectionMat.Inverse();

  float viewport[4];
  viewport[0] = view.x;
  viewport[1] = view.y;
  viewport[2] = view.w;
  viewport[3] = view.h;

  glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);

  int offset = 0;
  int dataSize = (sizeof(float) * 16);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &viewMat.m);

  offset += dataSize;
  dataSize = (sizeof(float) * 16);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &projectionMat.m);

  offset += dataSize;
  dataSize = (sizeof(float) * 16);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &projectionMatInverse.m);

  offset += dataSize;
  dataSize = (sizeof(float) * 4);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, &viewport[0]);

  Vec4 testVector(-1, -1, 90, 1);
  Vec4 testVector2(-1, -1, -10, 1);
  auto test1 = projectionMatInverse * testVector;
  auto test2 = viewMat.Inverse() * test1;
  auto test3 = projectionMat * testVector;
  auto test4 = projectionMat * testVector2;

  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Push viewport bit
  glPushAttrib(GL_VIEWPORT_BIT);
  // Setup frame buffer render
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  // Set Render Viewport
  glViewport(0, 0, frameBufferTextureDiffuse->GetWidth(),
             frameBufferTextureDiffuse->GetHeight());

  // Clear Background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Atatch buffer texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         frameBufferTextureDiffuse->GetOpenGLID(), 0);
  // Atatch buffer texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         mDepthTextureID, 0);

  // Some drivers report an error in this funciton, even though everything is
  // fine
  GL_GetError();
  auto fbError = GL_CheckFramebuffer();
  if (fbError != "") {
    LOG_ERROR(fbError);
  }
}

void RenderCamera::RenderFrameBufferTextureFinal(
    ComponentLightManager *lightMan, RSC_GLProgram *program) {
  lightMan->Render(frameBufferTextureDiffuse->GetOpenGLID(), mDepthTextureID,
                   frameBufferTextureFinal->GetOpenGLID(), view.w, view.h,
                   program);
  RenderFrameBufferTexture(frameBufferTextureFinal.get());
}
void RenderCamera::RenderFrameBufferTextureDiffuse() {
  RenderFrameBufferTexture(frameBufferTextureDiffuse.get());
}
void RenderCamera::RenderFrameBufferTexture(const RSC_Texture *tex) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  RSC_GLProgram::BindNULL();
  glBindVertexArray(0);

  // Back to initial viewport
  glPopAttrib();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  auto resolution = Resolution::GetResolution();
  glOrtho(0.0f, resolution.x, resolution.y, 0, 0, 1);  // 2D
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /*
  auto vres = Resolution::GetVirtualResolution();
  auto res = Resolution::GetResolution();
  auto ratioX = vres.x / res.x;
  auto ratioY = vres.y / res.y;
  */
  tex->Bind();
  float Left = 0;
  float Right = 1;  // * ratioX;
  // texture is upside down, invert top and bottom
  float Top = 1;  // * ratioY;
  float Bottom = 0;

  glBegin(GL_QUADS);
  glTexCoord2f(Left, Top);
  glVertex3f(0, 0, 0);

  glTexCoord2f(Right, Top);
  glVertex3f(resolution.x, 0, 0);

  glTexCoord2f(Right, Bottom);
  glVertex3f(resolution.x, resolution.y, 0);

  glTexCoord2f(Left, Bottom);
  glVertex3f(0, resolution.y, 0);
  glEnd();

  /*
  RSC_Texture::ExportTexture(mDepthTextureID, view.w, view.h, 1,
  GL_DEPTH_COMPONENT, "./test.png");
  */
}

/////////////////
// RenderManager//
/////////////////
bool OrderFrontToBack(RenderableObject *r1, RenderableObject *r2) {
  if (r1->GetDepth() <= r2->GetDepth()) {
    return true;
  }
  return false;
}
bool OrderBackToFront(RenderableObject *r1, RenderableObject *r2) {
  if (r1->GetDepth() <= r2->GetDepth()) {
    return false;
  }
  return true;
}

GLuint RenderManager::GlobalCameraUBO = 0;
GLuint RenderManager::GlobalProgramUBO = 0;

const GLuint RenderManager::CameraDataBindingIndex = 1;
const GLuint RenderManager::ProgramDataBindingIndex = 2;
const std::string RenderManager::defaultProgramTileName = "TileLayer.xml";
const std::string RenderManager::defaultProgramSpriteName = "SpriteBatch.xml";
const std::string RenderManager::defaultProgramLightName = "Light.xml";
const std::string RenderManager::defaultProgramImageName = "ImageLayer.xml";

RenderManager::RenderManager() : timeElapsed(0) {
  LoadDefaultShaders();

  listChange = false;
  nextTextID = 0;
}

void RenderManager::ProcessDrawCall(RenderableObject *obj,
                                    RenderCamera *camera) {
#ifdef DEBUG_MODE
  auto GLerror = GL_GetError();
  if (GLerror != "") {
    std::stringstream ss;
    ss << "GL Error '" << GLerror << "' Occured before rendering a "
       << obj->GetTypeString();
    LOG_ERROR(ss.str());
  }
#endif

  obj->Render(camera);

#ifdef DEBUG_MODE
  GLerror = GL_GetError();
  if (GLerror != "") {
    std::stringstream ss;
    ss << "GL Error '" << GLerror << "' Occured when rendering a "
       << obj->GetTypeString();
    LOG_ERROR(ss.str());
  }
#endif
}

void RenderManager::Render() {
  auto resolution = Resolution::GetResolution();
  glViewport(0, 0, resolution.x, resolution.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  timeElapsed += 1;

  glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
  float values[4];
  values[0] = timeElapsed;
  values[1] = 0;
  values[2] = resolution.x;
  values[3] = resolution.y;

  glBufferSubData(GL_UNIFORM_BUFFER, 0, (sizeof(float) * 4), &values);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  std::vector<RenderableObjectWorld *> worldOpaque;
  std::vector<RenderableObjectWorld *> worldTransparent;
  for (auto i = objectsWorld.begin(); i != objectsWorld.end(); i++) {
    if ((*i)->render) {
      if ((*i)->isTransparent()) {
        worldTransparent.push_back((*i));
      } else {
        worldOpaque.push_back((*i));
      }
    }
  }

  std::sort(worldOpaque.begin(), worldOpaque.end(), &OrderFrontToBack);
  std::sort(worldTransparent.begin(), worldTransparent.end(),
            &OrderBackToFront);

  if (!(mCameras).empty()) {
    for (auto camera = mCameras.begin(); camera != mCameras.end(); camera++) {
      (*camera)->Bind(GlobalCameraUBO);
      // Render opague objects
      // Write to depth
      glDepthMask(GL_TRUE);
      for (auto i = worldOpaque.begin(); i != worldOpaque.end(); i++) {
        ProcessDrawCall(*i, *camera);
      }

      // Render transparent objects
      // Do not write to depth
      glDepthMask(GL_FALSE);
      for (auto i = worldTransparent.begin(); i != worldTransparent.end();
           i++) {
        ProcessDrawCall(*i, *camera);
      }
      glDepthMask(GL_TRUE);

      // Maybe have per camera objects as well?
      // Per viewport objects?
      // used, for example, multiple HUDs in scplit-screen

      //(*camera)->RenderFrameBufferTextureDiffuse();
      (*camera)->RenderFrameBufferTextureFinal(
          &Kernel::stateMan.GetCurrentState()->comLightMan,
          &defaultProgramLight);
    }
  }

  // Screen objects are rendered after others, but do not use the camera's
  // matrix
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  RSC_GLProgram::BindNULL();
  glBindVertexArray(0);

  for (auto i = objectsScreen.begin(); i != objectsScreen.end(); i++) {
    if ((*i)->render) {
      (*i)->Render();
    }
  }

  ImGui::Render();
  ImGuiRender(ImGui::GetDrawData());
}

void RenderManager::AssignCameraUBO(RSC_GLProgram *program) {
  // Get program and uniform buffer handles
  GLuint programHandle = program->GetHandle();
  try {
    GLuint programUniformBlockHandle =
        program->GetUniformBlockHandle("CameraData");

    // Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle,
                          CameraDataBindingIndex);
  } catch (LEngineShaderProgramException e) {
  }
}

RenderSpriteBatch *RenderManager::GetSpriteBatch(const RSC_Texture *tex,
                                                 const RSC_Texture *texNormal,
                                                 const MAP_DEPTH &depth,
                                                 const int &numSprites) {
  auto textureMapIt = spriteBatchMap.find(depth);
  // If there isn't a map for this depth value,
  // create it and assign texturemapit to it
  if (textureMapIt == spriteBatchMap.end()) {
    spriteBatchMap[depth];
    textureMapIt = spriteBatchMap.find(depth);
  }

  auto textureNormalMapIt = textureMapIt->second.find(tex);
  // If there isn't a map for this normal texture
  // create it and assign textureNormalMapit to it
  if (textureNormalMapIt == textureMapIt->second.end()) {
    textureMapIt->second[tex];
    textureNormalMapIt = textureMapIt->second.find(tex);
  }

  auto spriteBatchVectorIt = textureNormalMapIt->second.find(texNormal);
  // If there isn't a vector for this texture name value, then
  // create it and assign spriteBatchVectorIt to it
  if (spriteBatchVectorIt == textureNormalMapIt->second.end()) {
    textureNormalMapIt->second[texNormal];
    spriteBatchVectorIt = textureNormalMapIt->second.find(texNormal);
  }

  RenderSpriteBatch *batch = NULL;
  for (auto vectorIt = spriteBatchVectorIt->second.begin();
       vectorIt != spriteBatchVectorIt->second.end(); vectorIt++) {
    if ((*vectorIt)->CanAddSprites(numSprites) == true) {
      batch = (*vectorIt).get();
      break;
    }
  }

  // If there isn't a spritebatch of appropriate size, create one and assign
  // batch to it
  if (batch == NULL) {
    // Max size 256
    spriteBatchVectorIt->second.push_back(std::unique_ptr<RenderSpriteBatch>(
        new RenderSpriteBatch(this, tex, texNormal, 256)));
    batch = (spriteBatchVectorIt->second.back()).get();
    batch->SetDepth(depth);
  }
  return batch;
}

void RenderManager::RemoveObjectScreen(RenderableObjectScreen *obj) {
  for (auto i = objectsScreen.begin(); i != objectsScreen.end(); i++) {
    if (obj == *i) {
      objectsScreen.erase(i);
      listChange = true;
      break;
    }
  }
}

void RenderManager::RemoveObjectWorld(RenderableObjectWorld *obj) {
  for (auto i = objectsWorld.begin(); i != objectsWorld.end(); i++) {
    if (obj == *i) {
      objectsWorld.erase(i);
      listChange = true;
      break;
    }
  }
}

void RenderManager::AddObjectWorld(RenderableObjectWorld *obj) {
  // Maybe use visitor pattern here to achieve double dispatch (?)
  objectsWorld.push_back(obj);
  listChange = true;

  // Set Correct Shader
  if (obj->GetShaderProgram() == NULL) {
    if (obj->type == RenderableObject::TYPE::SpriteBatch) {
      obj->SetShaderProgram(&defaultProgramSprite);
    }
    if (obj->type == RenderableObject::TYPE::TileLayer) {
      obj->SetShaderProgram(&defaultProgramTile);
    }
    if (obj->type == RenderableObject::TYPE::Image) {
      obj->SetShaderProgram(&defaultProgramImage);
    }
  }
}

void RenderManager::AddObjectScreen(RenderableObjectScreen *obj) {
  objectsScreen.push_back(obj);
  listChange = true;
}

bool RenderManager::AddCamera(RenderCamera *cam) {
  mCameras.insert(cam);
  return true;
}

void RenderManager::RemoveCamera(RenderCamera *cam) { mCameras.erase(cam); }

void RenderManager::ImGuiRender(ImDrawData *drawData) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  ImGuiState *guiState = &Kernel::guiState;
  ImGuiIO &io = ImGui::GetIO();
  int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0) return;
  drawData->ScaleClipRects(io.DisplayFramebufferScale);

  // Backup GL state
  GLint last_active_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  GLint last_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_array_buffer;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  GLint last_element_array_buffer;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
  GLint last_vertex_array;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
  GLint last_blend_src_rgb;
  glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src_rgb);
  GLint last_blend_dst_rgb;
  glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst_rgb);
  GLint last_blend_src_alpha;
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src_alpha);
  GLint last_blend_dst_alpha;
  glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst_alpha);
  GLint last_blend_equation_rgb;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
  GLint last_blend_equation_alpha;
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  // Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);

  // Setup viewport, projection matrix
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);

  guiState->shaderHandle->Bind();
  glUniform1i(guiState->attribLocationTex, 0);
  glUniformMatrix4fv(guiState->attribLocationProjMtx, 1, GL_FALSE,
                     guiState->projectionMatrix.m);
  glBindVertexArray(guiState->vaoHandle);

  for (int n = 0; n < drawData->CmdListsCount; n++) {
    const ImDrawList *cmd_list = drawData->CmdLists[n];
    const ImDrawIdx *idx_buffer_offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, guiState->vboHandle);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                 (const GLvoid *)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, guiState->elementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                 (const GLvoid *)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w),
                  (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(
            GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                      last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (last_enable_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (last_enable_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2],
             (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1],
            (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

std::unique_ptr<RSC_GLProgram> RenderManager::LoadShaderProgram(
    const std::string &vertName, const std::string &fragName) {
  std::unique_ptr<const RSC_GLShader> fragShader(std::make_unique<RSC_GLShader>(
      RSC_GLShader::LoadShaderFromFile(fragName), SHADER_FRAGMENT));
  std::unique_ptr<const RSC_GLShader> vertShader(std::make_unique<RSC_GLShader>(
      RSC_GLShader::LoadShaderFromFile(vertName), SHADER_VERTEX));

  if (fragShader->GetShaderID() != 0) {
    K_ShaderMan.LoadItem(fragName, fragShader);
  }
  if (vertShader->GetShaderID() != 0) {
    K_ShaderMan.LoadItem(vertName, vertShader);
  }

  std::unique_ptr<RSC_GLProgram> program = std::make_unique<RSC_GLProgram>();
  program->AddShader(K_ShaderMan.GetItem(fragName));
  program->AddShader(K_ShaderMan.GetItem(vertName));
  program->LinkProgram();
  program->Bind();

  return program;
}

void RenderManager::SetupUniformBuffers(RSC_GLProgram *program) {
  try {
    GLuint programHandle = program->GetHandle();
    GLuint programUniformBlockHandle =
        program->GetUniformBlockHandle("CameraData");
    // Bind program GPU buffer to the index
    glUniformBlockBinding(programHandle, programUniformBlockHandle,
                          CameraDataBindingIndex);
  } catch (LEngineShaderProgramException e) {
    // It's fine if gl can't link the uniform block
    // if the program doesn't use the program data block, the compiled code
    // won't have one
    // which will throw an error
    // LOG_WARN(e.what());
  }
  try {
    GLuint programHandle = program->GetHandle();
    GLuint programUniformBlockHandleProgramData =
        program->GetUniformBlockHandle("ProgramData");
    glUniformBlockBinding(programHandle, programUniformBlockHandleProgramData,
                          ProgramDataBindingIndex);
  } catch (LEngineShaderProgramException e) {
    // It's fine if gl can't link the uniform block
    // if the program doesn't use the program data block, the compiled code
    // won't have one
    // which will throw an error
    // LOG_WARN(e.what());
  }
}

void RenderManager::LoadDefaultShaders() {
  // Init gpu data if it hasn't been already
  if (GlobalCameraUBO == 0) {
    // Create memory location on GPU to store uniform camera data for ALL SHADER
    // PROGRAMS
    // The memory location ID is then sent to each individual camera so that the
    // cameras can bind
    // the needed data into the uniform buffer
    // This buffer stores three mat4 (view mat, proj matr and inverse) and a
    // vec4
    GLuint bufferSize = (((sizeof(float) * 16) * 3) + sizeof(float) * 4);
    glGenBuffers(1, &GlobalCameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalCameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
    // Bind generated CPU buffer to the index
    glBindBufferBase(GL_UNIFORM_BUFFER, CameraDataBindingIndex,
                     GlobalCameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Create memory location on GPU to store uniform data for ALL SHADER
    // PROGRAMS
    // The memory location ID is then sent to each individual camera so that the
    // cameras can bind
    // the needed data into the uniform buffer
    // This buffer stores the elapsed time
    bufferSize = (sizeof(float) * 4);
    glGenBuffers(1, &GlobalProgramUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, GlobalProgramUBO);
    glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
    // Bind generated CPU buffer to the index
    glBindBufferBase(GL_UNIFORM_BUFFER, ProgramDataBindingIndex,
                     GlobalProgramUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  auto name = defaultProgramTileName;
  auto shaderProgram = K_ShaderProgramMan.GetLoadItem(name, name);
  if (shaderProgram != NULL) {
    defaultProgramTile.CopyShadersFromProgram(shaderProgram);
    SetupUniformBuffers(&defaultProgramTile);
  }

  name = defaultProgramImageName;
  shaderProgram = K_ShaderProgramMan.GetLoadItem(name, name);
  if (shaderProgram != NULL) {
    defaultProgramImage.CopyShadersFromProgram(shaderProgram);
    SetupUniformBuffers(&defaultProgramImage);
  }

  name = defaultProgramSpriteName;
  shaderProgram = K_ShaderProgramMan.GetLoadItem(name, name);
  if (shaderProgram != NULL) {
    defaultProgramSprite.CopyShadersFromProgram(shaderProgram);
    SetupUniformBuffers(&defaultProgramSprite);
  }

  name = defaultProgramLightName;
  shaderProgram = K_ShaderProgramMan.GetLoadItem(name, name);
  if (shaderProgram != NULL) {
    defaultProgramLight.CopyShadersFromProgram(shaderProgram);
    SetupUniformBuffers(&defaultProgramLight);
    ComponentLightManager::BindLightUBO(&defaultProgramLight);
  }
}
