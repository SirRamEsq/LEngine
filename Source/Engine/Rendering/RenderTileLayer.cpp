#include "RenderTileLayer.h"
#include "../Kernel.h"

//////////////
// VAOWrapperTile//
//////////////
VAOWrapperTile::VAOWrapperTile(const unsigned int &maxSize)
    : vboMaxSize(maxSize),
      vboVertexSize(maxSize * sizeof(Vec2) * 4),  // 4 verticies per object
      vboTextureSize(maxSize * sizeof(Vec4) * 4),
      vboAnimationSize(maxSize * sizeof(Vec2) * 4),

      vboVertexArray(new Vec2[maxSize * 4]),
      vboTextureArray(new Vec4[maxSize * 4]),
      vboAnimationArray(new Vec2[maxSize * 4]) {
  // Vertex VBO
  glGenBuffers(1, &vboVertex);
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  //     Size of Buffer                           Pointer to data
  glBufferData(GL_ARRAY_BUFFER, vboVertexSize, vboVertexArray.get(),
               GL_STATIC_DRAW);

  // Texture VBO
  glGenBuffers(1, &vboTexture);
  glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
  glBufferData(GL_ARRAY_BUFFER, vboTextureSize, vboTextureArray.get(),
               GL_STATIC_DRAW);

  // Animation VBO
  glGenBuffers(1, &vboAnimation);
  glBindBuffer(GL_ARRAY_BUFFER, vboAnimation);
  glBufferData(GL_ARRAY_BUFFER, vboAnimationSize, vboAnimationArray.get(),
               GL_STATIC_DRAW);

  // Generate VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Bind Vertex to 0
  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  glVertexAttribPointer(0, vertexAttributeSize, vertexAttributeType, GL_FALSE,
                        0, NULL);

  // Bind Texture to 1
  glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
  glVertexAttribPointer(1, textureAttributeSize, textureAttributeType, GL_FALSE,
                        0, NULL);

  // Bind Animation to 2
  glBindBuffer(GL_ARRAY_BUFFER, vboAnimation);
  glVertexAttribPointer(2, animationAttributeSize, animationAttributeType,
                        GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
}

void VAOWrapperTile::UpdateGPU() {
  // upload vertexTexture array along with any changed data

  glBindBuffer(GL_ARRAY_BUFFER, vboTexture);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboTextureSize, vboTextureArray.get());

  glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboVertexSize, vboVertexArray.get());

  glBindBuffer(GL_ARRAY_BUFFER, vboAnimation);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboAnimationSize,
                  vboAnimationArray.get());
}

VAOWrapperTile::~VAOWrapperTile() {
  glDeleteBuffers(1, &vboVertex);
  glDeleteBuffers(1, &vboTexture);
  glDeleteBuffers(1, &vboAnimation);

  glDeleteVertexArrays(1, &vao);
}

////////////////////////
// RenderTileLayer//
////////////////////////

RenderTileLayer::RenderTileLayer(RenderManager *rm, const TiledTileLayer *l)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::TileLayer),
      vao(l->tileWidth * l->tileHeight) {
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
  BuildVAO();

  AddToRenderManager();
}

RenderTileLayer::~RenderTileLayer() {}

/// \TODO have ONE COLOR for the whole map, not per tile
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
  float textureWidth = tiledSet->GetTexture()->GetWidth();
  float textureHeight = tiledSet->GetTexture()->GetHeight();
  GID gid;

  Vec4 color;
  color.x = 1.0f;
  color.y = 1.0f;
  color.z = 1.0f;
  color.w = layer->GetAlpha();

  translate.x = x * 16;
  translate.y = y * 16;

  gid = layer->GetGID(x, y);

  vao.GetVertexArray()[vertexIndex] = topLeftVertex + translate;
  vao.GetVertexArray()[vertexIndex + 1] = topRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex + translate;

  const LAnimation *animation = tiledSet->GetAnimationDataFromGID(gid);
  if (animation != NULL) {
    animationVertex.x = animation->GetSpeed();
    animationVertex.y = animation->NumberOfImages();
  }

  tiledSet->GetTextureCoordinatesFromGID(gid, leftTex, rightTex, topTex,
                                         bottomTex);
  Vec4 topLeftTex(leftTex, topTex, textureWidth, textureHeight);
  Vec4 topRightTex(rightTex, topTex, textureWidth, textureHeight);
  Vec4 bottomLeftTex(rightTex, bottomTex, textureWidth, textureHeight);
  Vec4 bottomRightTex(leftTex, bottomTex, textureWidth, textureHeight);

  vao.GetTextureArray()[vertexIndex] = topLeftTex;
  vao.GetTextureArray()[vertexIndex + 1] = topRightTex;
  vao.GetTextureArray()[vertexIndex + 2] = bottomLeftTex;
  vao.GetTextureArray()[vertexIndex + 3] = bottomRightTex;

  vao.GetAnimationArray()[vertexIndex] = animationVertex;
  vao.GetAnimationArray()[vertexIndex + 1] = animationVertex;
  vao.GetAnimationArray()[vertexIndex + 2] = animationVertex;
  vao.GetAnimationArray()[vertexIndex + 3] = animationVertex;
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
    glUniform4fv(program->GetUniformLocation("layerColor"), 1, colors);

  glBindVertexArray(vao.GetVAOID());
  tiledSet->GetTexture()->Bind();
  // draw points 0-4 from the currently bound VAO with current in-use shader
  // render full number of tiles
  glDrawArrays(GL_QUADS, 0, layer->tileWidth * layer->tileHeight * 4);
}
