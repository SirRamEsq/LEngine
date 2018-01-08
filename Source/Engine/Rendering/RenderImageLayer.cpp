#include "RenderImageLayer.h"
#include "../RenderManager.h"

RenderImageLayer::RenderImageLayer(RenderManager *rm, TiledImageLayer *l)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::Image),
      layer(l),
      vao(VAO_TEXTURE | VAO_COLOR, 1) {
  SetDepth(l->GetDepth());
  render = true;
  AddToRenderManager();
  BuildVAO(Rect(0, 0, 0, 0));
}

void RenderImageLayer::BuildVAO(Rect camera) {
  const RSC_Texture *tex = layer->GetTexture();

  Vec2 topLeftVertex(camera.GetLeft(), camera.GetTop());
  Vec2 topRightVertex(camera.GetRight(), camera.GetTop());
  Vec2 bottomRightVertex(camera.GetRight(), camera.GetBottom());
  Vec2 bottomLeftVertex(camera.GetLeft(), camera.GetBottom());

  Vec4 color;
  color.x = 1.0f;
  color.y = 1.0f;
  color.z = 1.0f;
  color.w = layer->GetAlpha();

  auto parallax = layer->GetParallax();
  float parallaxX = parallax.x;
  float parallaxY = parallax.y;

  auto translate = layer->GetOffset();

  int texW = tex->GetWidth();
  int texH = tex->GetHeight();

  auto repeatX = layer->GetRepeatX();
  auto repeatY = layer->GetRepeatY();
  auto stretchX = layer->GetStretchToMapX();
  auto stretchY = layer->GetStretchToMapY();

  // Stretch/fit bg to each corner of the camera viewport by default
  float leftTex = 0.0;
  float rightTex = 1.0f;
  float topTex = 0.0f;
  float bottomTex = 1.0f;

  if (repeatX) {
    leftTex = (float(floor(camera.GetLeft() * parallaxX) + translate.x) /
               (float)texW);
    rightTex = (float(floor(camera.GetRight() * parallaxX) + translate.x) /
                (float)texW);
  }
  if (repeatY) {
    topTex =
        (float(floor(camera.GetTop() * parallaxY) + translate.y) / (float)texH);
    bottomTex = (float(floor(camera.GetBottom() * parallaxY) + translate.y) /
                 (float)texH);
  }
  // Fit image to left and right sides of the map
  if (stretchX) {
    float mapW = layer->pixelWidth;
    leftTex = float(camera.GetLeft()) / mapW;
    rightTex = float(camera.GetRight()) / mapW;
  }
  // Fit image to top and bottom sides of the map
  if (stretchY) {
    float mapH = layer->pixelHeight;
    topTex = float(camera.GetTop()) / mapH;
    bottomTex = float(camera.GetBottom()) / mapH;
  }

  Vec2 topLeftTex(leftTex, topTex);
  Vec2 topRightTex(rightTex, topTex);
  Vec2 bottomLeftTex(leftTex, bottomTex);
  Vec2 bottomRightTex(rightTex, bottomTex);

  int vertexIndex = 0;
  vao.GetVertexArray()[vertexIndex] = topLeftVertex;
  vao.GetVertexArray()[vertexIndex + 1] = topRightVertex;
  vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex;
  vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex;

  vao.GetTextureArray()[vertexIndex] = topLeftTex;
  vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
  vao.GetTextureArray()[vertexIndex + 2] = bottomRightTex;
  vao.GetTextureArray()[vertexIndex + 3] = bottomLeftTex;

  vao.GetColorArray()[vertexIndex] = color;
  vao.GetColorArray()[vertexIndex + 1] = color;
  vao.GetColorArray()[vertexIndex + 2] = color;
  vao.GetColorArray()[vertexIndex + 3] = color;

  vao.UpdateGPU();
}

RenderImageLayer::~RenderImageLayer() {}

bool RenderImageLayer::isTransparent(){
	if(color.w == 1.0){
		return false;
	}
	return true;
}

void RenderImageLayer::Render(const RenderCamera *camera,
                              const RSC_GLProgram *program) {
  // Store old texture wrap settings
  GLint oldParamS;
  GLint oldParamT;
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &oldParamS);
  glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &oldParamT);

  // Set new texture wrap settings
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Calculate a projection matrix here based on distance from the camera?
  BuildVAO(camera->GetView());

  // Bind everything
  program->Bind();
  vao.Bind();
  layer->GetTexture()->Bind();
  float depth = GetDepth();
  glUniform1fv(program->GetUniformLocation("depth"), 1, &depth);
  glDrawArrays(GL_QUADS, 0, 4);

  // Restore old texture Wrap Settings
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, oldParamS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, oldParamT);
}
