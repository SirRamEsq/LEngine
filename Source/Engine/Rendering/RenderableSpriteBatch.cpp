#include "RenderableSpriteBatch.h"
#include "../Kernel.h"
#include "../RenderManager.h"
#include "../Resources/RSC_Texture.h"

////////////////////
// RenderableSprite//
////////////////////

RenderableSpriteBatch::Sprite::Sprite(const Sprite &other)
    : texture(other.texture),
      textureNormal(other.textureNormal),
      depth(other.depth),
      offset(other.offset) {
  manager = other.manager;
  auto rm = manager;
  spriteBatch = rm->GetSpriteBatch(texture, textureNormal, depth, 1);
  spriteBatch->AddSprite(this);

  data.color = other.data.color;

  scaleX = other.scaleX;
  scaleY = other.scaleY;
  rotation = other.rotation;

  data.scalingRotation.x = scaleX;
  data.scalingRotation.y = scaleY;
  data.scalingRotation.z = rotation;

  data.translate = other.data.translate;

  isActive = other.isActive;
}

RenderableSpriteBatch::Sprite::Sprite(RenderManager *rm, const RSC_Sprite *spr,
                                      MAP_DEPTH d, Vec2 off)
    : texture(K_TextureMan.GetLoadItem(spr->GetTextureName())),
      textureNormal(NULL),
      manager(rm),
      depth(d),

      offset(off) {
  spriteBatch = rm->GetSpriteBatch(texture, textureNormal, depth, 1);
  spriteBatch->AddSprite(this);

  data.color.x = 1.0f;
  data.color.y = 1.0f;
  data.color.z = 1.0f;
  data.color.w = 1.0f;

  scaleX = 1.0f;
  scaleY = 1.0f;
  rotation = 0.0f;

  data.scalingRotation.x = scaleX;
  data.scalingRotation.y = scaleY;
  data.scalingRotation.z = rotation;

  data.translate.x = 0;
  data.translate.y = 0;

  isActive = true;
}

RenderableSpriteBatch::Sprite::~Sprite() { spriteBatch->DeleteSprite(this); }

void RenderableSpriteBatch::Sprite::SetDepth(MAP_DEPTH d) {
  if (depth == d) {
    return;
  }
  depth = d;
  spriteBatch->DeleteSprite(this);
  spriteBatch = manager->GetSpriteBatch(texture, textureNormal, depth, 1);
  spriteBatch->AddSprite(this);
}
MAP_DEPTH RenderableSpriteBatch::Sprite::GetDepth() const { return depth; }

/////////////////////
// RenderableSpriteBatch//
/////////////////////

RenderableSpriteBatch::RenderableSpriteBatch(RenderManager *rm,
                                             const RSC_Texture *tex,
                                             const RSC_Texture *texNormal,
                                             const unsigned int &maxSize)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::SpriteBatch),
      maxSprites(maxSize),
      texture(tex),
      textureNormal(texNormal),
      vao((VAO_TEXTURE | VAO_COLOR | VAO_EXTRA | VAO_SCALINGROTATION),
          maxSize) {
  currentSize = 0;
  AddToRenderManager();
}

bool RenderableSpriteBatch::CanAddSprites(const int &numSprites) {
  return (numSprites + currentSize) < maxSprites;
}

void RenderableSpriteBatch::AddSprite(RenderableSpriteBatch::Sprite *sprite) {
  sprites.insert(sprite);
  currentSize++;
}

void RenderableSpriteBatch::DeleteSprite(
    RenderableSpriteBatch::Sprite *sprite) {
  auto spriteIt = sprites.find(sprite);
  if (spriteIt == sprites.end()) {
    return;
  }

  sprites.erase(spriteIt);
  currentSize--;
}

void RenderableSpriteBatch::Render(const RenderCamera *camera,
                                   const RSC_GLProgram *program) {
  vao.Bind();
  unsigned int numberOfSprites = 0;
  unsigned int vertexIndex = 0;
  RenderableSpriteBatch::Sprite *sprite;
  for (auto i = sprites.begin(); i != sprites.end(); i++) {
    // Create array of correct values
    // if sprite isn't active, do not add one from 'numberOfSprites' or add its
    // data to the array; ignore it
    sprite = (*i);
    if (sprite->isActive == false) {
      continue;
    }
    numberOfSprites++;
    std::string ss;

    vao.GetVertexArray()[vertexIndex] = sprite->data.vertex1;
    vao.GetVertexArray()[vertexIndex + 1] = sprite->data.vertex2;
    vao.GetVertexArray()[vertexIndex + 2] = sprite->data.vertex3;
    vao.GetVertexArray()[vertexIndex + 3] = sprite->data.vertex4;

    vao.GetTextureArray()[vertexIndex] = sprite->data.texture1;
    vao.GetTextureArray()[vertexIndex + 1] = sprite->data.texture2;
    vao.GetTextureArray()[vertexIndex + 2] = sprite->data.texture3;
    vao.GetTextureArray()[vertexIndex + 3] = sprite->data.texture4;

    vao.GetColorArray()[vertexIndex] = sprite->data.color;
    vao.GetColorArray()[vertexIndex + 1] = sprite->data.color;
    vao.GetColorArray()[vertexIndex + 2] = sprite->data.color;
    vao.GetColorArray()[vertexIndex + 3] = sprite->data.color;

    vao.GetScalingRotationArray()[vertexIndex] = sprite->data.scalingRotation;
    vao.GetScalingRotationArray()[vertexIndex + 1] =
        sprite->data.scalingRotation;
    vao.GetScalingRotationArray()[vertexIndex + 2] =
        sprite->data.scalingRotation;
    vao.GetScalingRotationArray()[vertexIndex + 3] =
        sprite->data.scalingRotation;

    vao.GetExtraArray()[vertexIndex] = sprite->data.translate;
    vao.GetExtraArray()[vertexIndex + 1] = sprite->data.translate;
    vao.GetExtraArray()[vertexIndex + 2] = sprite->data.translate;
    vao.GetExtraArray()[vertexIndex + 3] = sprite->data.translate;

    vertexIndex += 4;
  }

  vao.UpdateGPU();

  program->Bind();

  float depth = GetDepth();
  glUniform1fv(program->GetUniformLocation("depth"), 1, &depth);

  if (texture != NULL) {
    texture->Bind();
  }
  if (textureNormal != NULL) {
    RSC_Texture::SetActiveTexture(GL_TEXTURE1);
    textureNormal->Bind();
    RSC_Texture::SetActiveTexture(GL_TEXTURE0);
  }

  glDrawArrays(GL_QUADS, 0, numberOfSprites * 4);
}

bool RenderableSpriteBatch::isTransparent() { return true; }
