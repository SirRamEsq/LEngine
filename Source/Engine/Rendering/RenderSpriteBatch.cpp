#include "RenderSpriteBatch.h"
#include "../Kernel.h"

////////////////////
// RenderableSprite//
////////////////////

RenderSpriteBatch::Sprite::Sprite(RenderManager *rm, const std::string &texture,
                                  const unsigned int &w, const unsigned int &h,
                                  const MAP_DEPTH &d, const Vec2 &off)
    : textureName(texture),
      textureWidth(w),
      textureHeight(h),
      depth(d),
      offset(off) {
  spriteBatch = rm->GetSpriteBatch(textureName, depth, 1);
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

RenderSpriteBatch::Sprite::~Sprite() { spriteBatch->DeleteSprite(this); }

/////////////////////
// RenderSpriteBatch//
/////////////////////

RenderSpriteBatch::RenderSpriteBatch(RenderManager *rm, const std::string &tex,
                                     const unsigned int &maxSize)
    : RenderableObjectWorld(rm, RenderableObject::TYPE::SpriteBatch),
      maxSprites(maxSize),
      textureName(tex),
      vao((VAO_TEXTURE | VAO_COLOR | VAO_EXTRA | VAO_SCALINGROTATION),
          maxSize) {
  currentSize = 0;
  // this dependency on the Kernel is ok, this is just to grab a resource
  texture = K_TextureMan.GetItem(textureName);
  if (texture == NULL) {
    std::stringstream ss;
    ss << "ERROR: RenderSpriteBatch; Couldn't find texture named: "
       << textureName;
    LOG_INFO(ss.str());
  }
  AddToRenderManager();
}

bool RenderSpriteBatch::CanAddSprites(const int &numSprites) {
  return (numSprites + currentSize) < maxSprites;
}

void RenderSpriteBatch::AddSprite(RenderSpriteBatch::Sprite *sprite) {
  sprites.insert(sprite);
  currentSize++;
}

void RenderSpriteBatch::DeleteSprite(RenderSpriteBatch::Sprite *sprite) {
  auto spriteIt = sprites.find(sprite);
  if (spriteIt == sprites.end()) {
    return;
  }

  sprites.erase(spriteIt);
  currentSize--;
}

void RenderSpriteBatch::Render(const RenderCamera *camera,
                               const RSC_GLProgram *program) {
  vao.Bind();
  unsigned int numberOfSprites = 0;
  unsigned int vertexIndex = 0;
  RenderSpriteBatch::Sprite *sprite;
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
  glDrawArrays(GL_QUADS, 0, numberOfSprites * 4);
}

bool RenderSpriteBatch::isTransparent() { return true; }
