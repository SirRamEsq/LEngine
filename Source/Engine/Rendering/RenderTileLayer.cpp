#include "RenderTileLayer.h"
#include "../Kernel.h"


RenderTileLayer::RenderTileLayer(RenderManager *rm, const TiledTileLayer *l)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::TileLayer),
      vao((VAO_TEXTURE), l->tileWidth * l->tileHeight) {
  SetDepth(l->GetDepth());
  layer = l;

  color.a = l->GetAlpha();
  animatedRefreshRateTimer = l->GetAnimationRate();

  if (animatedRefreshRateTimer == 0) {
    animated = false;
  } else {
    animated = true;
  }

  // Get Texture and TiledSet to be used
  tiledSet = layer->GetTiledSet();
  textureWidth = tiledSet->GetTexture()->GetWidth();
  textureHeight = tiledSet->GetTexture()->GetHeight();
  BuildVAO();

  AddToRenderManager();
}

RenderTileLayer::~RenderTileLayer() {}

void RenderTileLayer::BuildVAOTile(unsigned int x, unsigned int y) {
  unsigned int vertexIndex = ((y * layer->tileWidth) + x) * 4;
  Vec2 translate;

  Vec2 topLeftVertex(0.0f, 0.0f);
  Vec2 topRightVertex(16.0f, 0.0f);
  Vec2 bottomRightVertex(16.0f, 16.0f);
  Vec2 bottomLeftVertex(0.0f, 16.0f);

  // Default y is '1' because '0%0' is undefined
  Vec2 animationVertex(0, 1);

  float topTex, rightTex, leftTex, bottomTex;
  GID gid;

  translate.x = x * 16;
  translate.y = y * 16;

  gid = layer->GetGID(x, y);

  vao.GetVertexArray()[vertexIndex] = topLeftVertex + translate;
  vao.GetVertexArray()[vertexIndex + 1] = topRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex + translate;

  /*
  const LAnimation *animation = tiledSet->GetAnimationDataFromGID(gid);
  if (animation != NULL) {
    animationVertex.x = animation->GetSpeed();
    animationVertex.y = animation->NumberOfImages();
  }
  */

  tiledSet->GetTextureCoordinatesFromGID(gid, leftTex, rightTex, topTex,
                                         bottomTex);
  Vec2 topLeftTex(leftTex, topTex);
  Vec2 topRightTex(rightTex, topTex);
  Vec2 bottomLeftTex(rightTex, bottomTex);
  Vec2 bottomRightTex(leftTex, bottomTex);

  vao.GetTextureArray()[vertexIndex] = topLeftTex;
  vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
  vao.GetTextureArray()[vertexIndex + 2] = bottomLeftTex;
  vao.GetTextureArray()[vertexIndex + 3] = bottomRightTex;

  // vao.GetAnimationArray()[vertexIndex] = animationVertex;
  // vao.GetAnimationArray()[vertexIndex + 1] = animationVertex;
  // vao.GetAnimationArray()[vertexIndex + 2] = animationVertex;
  // vao.GetAnimationArray()[vertexIndex + 3] = animationVertex;
}

void RenderTileLayer::BuildVAO() {
  for (unsigned int x = 0; x < layer->tileWidth; x++) {
    for (unsigned int y = 0; y < layer->tileHeight; y++) {
      BuildVAOTile(x, y);
    }
  }
  vao.UpdateGPU();
}

void RenderTileLayer::BuildVAOArea(Rect area) {
  for (unsigned int x = area.GetLeft(); x <= area.GetRight(); x++) {
    for (unsigned int y = area.GetTop(); y <= area.GetBottom(); y++) {
      BuildVAOTile(x, y);
    }
  }
  // must call vao.UpdateGPU() when done!
}

void RenderTileLayer::Render(const RenderCamera *camera,
                             const RSC_GLProgram *program) {
  /// \TODO divide tileMaps into chunks and render only visible chunks

  program->Bind();
  if (layer->updatedAreas.size() > 0) {
    for (auto i = layer->updatedAreas.begin(); i != layer->updatedAreas.end();
         i++) {
      BuildVAOArea((*i));
    }
    // not the best practice, clearing out the vector from the renderer, but it
    // works
    layer->updatedAreas.clear();
    vao.UpdateGPU();
  }

  float colors[4];
  colors[0] = 1.0;
  colors[1] = 1.0;
  colors[2] = 1.0;
  colors[3] = layer->GetAlpha();
  float depth = GetDepth();
  float textureDimensions[2];
  textureDimensions[0] = textureWidth;
  textureDimensions[1] = textureHeight;
  glUniform4fv(program->GetUniformLocation("layerColor"), 1, colors);
  glUniform2fv(program->GetUniformLocation("textureDimensions"), 1,
               textureDimensions);
  glUniform1fv(program->GetUniformLocation("depth"), 1, &depth);

  vao.Bind();
  tiledSet->GetTexture()->Bind();
  // draw points 0-4 from the currently bound VAO with current in-use shader
  // render full number of tiles
  glDrawArrays(GL_QUADS, 0, layer->tileWidth * layer->tileHeight * 4);
}

bool RenderTileLayer::isTransparent() {
  if (layer->GetAlpha() == 1.0f) {
    return false;
  }
  return true;
}
