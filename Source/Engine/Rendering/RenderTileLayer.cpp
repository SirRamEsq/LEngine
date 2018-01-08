#include "RenderTileLayer.h"
#include "../Kernel.h"
#include <algorithm>

RenderTileLayer::RenderTileLayer(RenderManager *rm, const TiledTileLayer *l)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::TileLayer),
      vao((VAO_TEXTURE), l->tileWidth * l->tileHeight) {
  SetDepth(l->GetDepth());
  layer = l;

  if (l == NULL) {
    LOG_ERROR("LAYER IS NULL");
  }

  color.w = l->GetAlpha();
  animatedRefreshRateTimer = l->GetAnimationRate();

  if (animatedRefreshRateTimer == 0) {
    animated = false;
  } else {
    animated = true;
  }

  // Get Texture and TiledSet to be used
  tiledSet = layer->GetTiledSet();
  auto tex = tiledSet->GetTexture();
  textureWidth = tex->GetWidth();
  textureHeight = tex->GetHeight();
  BuildVAO();

  AddToRenderManager();
}

RenderTileLayer::~RenderTileLayer() {}

void RenderTileLayer::BuildVAOTile(unsigned int x, unsigned int y) {
  auto vertexIndex = CalculateVertexIndex(x, y);

  Vec2 topLeftVertex(0.0f, 0.0f);
  Vec2 topRightVertex(16.0f, 0.0f);
  Vec2 bottomRightVertex(16.0f, 16.0f);
  Vec2 bottomLeftVertex(0.0f, 16.0f);

  Vec2 translate;
  translate.x = x * 16;
  translate.y = y * 16;

  vao.GetVertexArray()[vertexIndex] = topLeftVertex + translate;
  vao.GetVertexArray()[vertexIndex + 1] = topRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 2] = bottomRightVertex + translate;
  vao.GetVertexArray()[vertexIndex + 3] = bottomLeftVertex + translate;
  auto gid = layer->GetGID(x, y);
  SetTileTexture(vertexIndex, gid);
}

unsigned int RenderTileLayer::CalculateVertexIndex(unsigned int x,
                                                   unsigned int y) {
  return ((y * layer->tileWidth) + x) * 4;
}

void RenderTileLayer::SetTileTexture(unsigned int vertexIndex, GID gid) {
  float topTex, rightTex, leftTex, bottomTex;
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
  for (unsigned int x = area.GetLeft(); x < area.GetRight(); x++) {
    for (unsigned int y = area.GetTop(); y < area.GetBottom(); y++) {
      BuildVAOTile(x, y);
    }
  }
  // must call vao.UpdateGPU() when done!
}

bool RenderTileLayer::UpdateTileAnimations(const RenderCamera *camera) {
  bool dataUpdated = false;
  auto animations = tiledSet->GetTileAnimations();
  if (animations->empty()) {
    // nothing to do, no data updated
    return dataUpdated;
  }

  // cache results
  std::map<GID, GID> animationCache;

  auto time = renderManager->GetTimeElapsed();
  auto view = camera->GetView();
  int left = view.GetLeft() / 16;
  int right = (view.GetRight() / 16) + 1;
  int top = view.GetTop() / 16;
  int bottom = (view.GetBottom() / 16) + 1;
  left = std::max(0, left);
  right = std::min(int(layer->tileWidth), right);
  top = std::max(0, top);
  bottom = std::min(int(layer->tileHeight), bottom);

  for (auto x = left; x != right; x++) {
    for (auto y = top; y != bottom; y++) {
      GID tileImage = 0;
      auto gid = layer->GetGID(x, y);
      auto cached = animationCache.find(gid);
      if (cached == animationCache.end()) {
        auto it = animations->find(gid);
        if (it != animations->end()) {
          auto ani = it->second;
          // update cache
          int range = time % ani.Length();

          for (auto i = ani.frames.begin(); i != ani.frames.end(); i++) {
            range -= i->length;
            if (range < 0) {
              tileImage = i->tileID;
              animationCache[gid] = tileImage;
			  break;
            }
          }
        } else {
          continue;
        }
      } else {
        tileImage = cached->second;
      }
      auto index = CalculateVertexIndex(x, y);
      SetTileTexture(index, tileImage);
      dataUpdated = true;
    }
  }
  return dataUpdated;
}

void RenderTileLayer::Render(const RenderCamera *camera,
                             const RSC_GLProgram *program) {
  if (tiledSet == NULL) {
    return;
  }
  /// \TODO divide tileMaps into chunks and render only visible chunks

  bool updateGPU = UpdateTileAnimations(camera);
  if (layer->updatedAreas.size() > 0) {
    for (auto i = layer->updatedAreas.begin(); i != layer->updatedAreas.end();
         i++) {
      BuildVAOArea((*i));
    }
    // not the best practice, clearing out the vector from the renderer, but it
    // works
    layer->updatedAreas.clear();
    updateGPU = true;
  }

  if (updateGPU) {
    vao.UpdateGPU();
  }

  program->Bind();

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
