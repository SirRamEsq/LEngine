#include "CompSprite.h"
#include "../Kernel.h"

AnimationData::AnimationData(const TAnimationMap *aniMap) : animations(aniMap) {
  animate = true;
  currentTime = 0;
  currentImageIndex = 0;
  animationSpeed = 0.0f;
  currentAnimation = NULL;
  currentAnimationName = "";
  SetAnimation(animations->begin()->first);
}

void AnimationData::Update() {
  if (!animate) {
    return;
  }

  currentTime += animationSpeed;
  if (currentTime >= maxTime) {
    currentTime -= maxTime;
  }
  currentImageIndex = currentAnimation->GetFrameFromTimeElapsed(currentTime);
}

bool AnimationData::SetImageIndex(const int &imageIndex) {
  if (imageIndex >= maxFrames) {
    return false;
  }

  currentTime = currentAnimation->GetTime(imageIndex);
  currentImageIndex = imageIndex;

  return true;
}

bool AnimationData::SetAnimation(const std::string &aniName) {
  if (currentAnimationName == aniName) {
    return true;
  }
  auto animation = animations->find(aniName);
  if (animation == animations->end()) {
    return false;
  }

  currentAnimationName = aniName;
  currentTime = 0.0f;
  currentImageIndex = 0;
  maxFrames = animation->second.NumberOfImages();
  maxTime = animation->second.GetMaxTime();
  animationSpeed = animation->second.GetSpeed();
  currentAnimation = &animation->second;
  defaultAnimationSpeed = animationSpeed;

  return true;
}

ComponentSprite::ComponentSprite(EID id, ComponentPosition *pos,
                                 RenderManager *r,
                                 ComponentSpriteManager *manager)
    : BaseComponent(id, manager) {
  rm = r;
  mEntityID = id;
  myPos = pos;
  mNumberOfLoadedSprites = 0;
}

ComponentSprite::~ComponentSprite() {}

void ComponentSprite::Update() {
  AnimationData *animation = NULL;
  RenderSpriteBatch::Sprite *renderSprite = NULL;
  const RSC_Sprite *spriteData = NULL;

  Coord2df pos = myPos->GetPositionWorld();

  unsigned int textureW = 0;
  unsigned int textureH = 0;

  // Update animationData and RenderagbleSprite info
  for (int it = 0; it != mNumberOfLoadedSprites; it++) {
    renderSprite = mRenderableSprites[it].get();
    if (renderSprite->isActive == false) {
      continue;
    }

    animation = &mAnimationData[it];
    spriteData = mSprites[it];

    animation->Update();

    auto ani = spriteData->GetAnimation(animation->GetAnimation());
    auto index = animation->GetImageIndex();

    // Top Left
    renderSprite->data.texture1.x = ani->GetUVLeft(index);
    renderSprite->data.texture1.y = ani->GetUVTop(index);

    // Top Right
    renderSprite->data.texture2.x = ani->GetUVRight(index);
    renderSprite->data.texture2.y = ani->GetUVTop(index);

    // Bottom Right
    renderSprite->data.texture3.x = ani->GetUVRight(index);
    renderSprite->data.texture3.y = ani->GetUVBottom(index);

    // Bottom Left
    renderSprite->data.texture4.x = ani->GetUVLeft(index);
    renderSprite->data.texture4.y = ani->GetUVBottom(index);

    Vec2 temp(0, 0);
    temp.x = renderSprite->data.vertexOrigin1.x;
    temp.y = renderSprite->data.vertexOrigin1.y;
    renderSprite->data.vertex1 = temp;

    temp.x = renderSprite->data.vertexOrigin2.x;
    temp.y = renderSprite->data.vertexOrigin2.y;
    renderSprite->data.vertex2 = temp;

    temp.x = renderSprite->data.vertexOrigin3.x;
    temp.y = renderSprite->data.vertexOrigin3.y;
    renderSprite->data.vertex3 = temp;

    temp.x = renderSprite->data.vertexOrigin4.x;
    temp.y = renderSprite->data.vertexOrigin4.y;
    renderSprite->data.vertex4 = temp;

    renderSprite->data.scalingRotation.x = renderSprite->scaleX;
    renderSprite->data.scalingRotation.y = renderSprite->scaleY;
    renderSprite->data.scalingRotation.z = renderSprite->rotation;

    // Render Using only full integers for translation to get that pixel-perfect
    // look
    int xPos = floor(pos.x + renderSprite->offset.x + 0.5f);
    int yPos = floor(pos.y + renderSprite->offset.y + 0.5f);
    renderSprite->data.translate.x = xPos;
    renderSprite->data.translate.y = yPos;
  }
}

void ComponentSprite::CalculateVerticies(int index) {
  LOrigin origin = mSprites[index]->GetOrigin();
  float spriteWidth = mSprites[index]->GetWidth();
  float spriteHeight = mSprites[index]->GetHeight();

  int spriteWidthHalf = spriteWidth / 2;
  int spriteHeightHalf = spriteHeight / 2;

  RenderSpriteBatch::Sprite *rSprite = mRenderableSprites[index].get();

  switch (origin) {
    case L_ORIGIN_CENTER:
      rSprite->data.vertexOrigin1.x = spriteWidthHalf * -1;
      rSprite->data.vertexOrigin1.y = spriteHeightHalf * -1;  // Top Left
      rSprite->data.vertexOrigin2.x = spriteWidthHalf;
      rSprite->data.vertexOrigin2.y = spriteHeightHalf * -1;  // Top Right
      rSprite->data.vertexOrigin3.x = spriteWidthHalf;
      rSprite->data.vertexOrigin3.y = spriteHeightHalf;  // Bottom Right
      rSprite->data.vertexOrigin4.x = spriteWidthHalf * -1;
      rSprite->data.vertexOrigin4.y = spriteHeightHalf;  // Bottom Left
      break;

    case L_ORIGIN_TOP_LEFT:
      rSprite->data.vertexOrigin1.x = 0;
      rSprite->data.vertexOrigin1.y = 0;
      rSprite->data.vertexOrigin2.x = spriteWidth;
      rSprite->data.vertexOrigin2.y = 0;
      rSprite->data.vertexOrigin3.x = spriteWidth;
      rSprite->data.vertexOrigin3.y = spriteHeight;
      rSprite->data.vertexOrigin4.x = 0;
      rSprite->data.vertexOrigin4.y = spriteHeight;
      break;

    case L_ORIGIN_TOP_RIGHT:
      rSprite->data.vertexOrigin1.x = spriteWidth;
      rSprite->data.vertexOrigin1.y = 0;
      rSprite->data.vertexOrigin2.x = 0;
      rSprite->data.vertexOrigin2.y = 0;
      rSprite->data.vertexOrigin3.x = 0;
      rSprite->data.vertexOrigin3.y = spriteHeight;
      rSprite->data.vertexOrigin4.x = spriteWidth;
      rSprite->data.vertexOrigin4.y = spriteHeight;
      break;

    case L_ORIGIN_BOTTOM_RIGHT:
      rSprite->data.vertexOrigin1.x = spriteWidth;
      rSprite->data.vertexOrigin1.y = spriteHeight;
      rSprite->data.vertexOrigin2.x = 0;
      rSprite->data.vertexOrigin2.y = spriteHeight;
      rSprite->data.vertexOrigin3.x = 0;
      rSprite->data.vertexOrigin3.y = 0;
      rSprite->data.vertexOrigin4.x = spriteWidth;
      rSprite->data.vertexOrigin4.y = 0;
      break;

    case L_ORIGIN_BOTTOM_LEFT:
      rSprite->data.vertexOrigin1.x = 0;
      rSprite->data.vertexOrigin1.y = spriteHeight;
      rSprite->data.vertexOrigin2.x = spriteWidth;
      rSprite->data.vertexOrigin2.y = spriteHeight;
      rSprite->data.vertexOrigin3.x = spriteWidth;
      rSprite->data.vertexOrigin3.y = 0;
      rSprite->data.vertexOrigin4.x = 0;
      rSprite->data.vertexOrigin4.y = 0;
      break;
  }
}

bool ComponentSprite::SpriteExists(int index) {
  if ((index < 0) || (index >= mNumberOfLoadedSprites)) {
    std::stringstream ss;
    ss << "Sprite Index out of range: Sprite Index '" << index
       << "' doesn't exist for entity with ID " << mEntityID;
    LOG_ERROR(ss.str());
    return false;
  }
  return true;
}

void ComponentSprite::SetRotation(int index, float rotation) {
  if (!SpriteExists(index)) {
    return;
  }
  mRenderableSprites[index].get()->rotation = rotation;
}
void ComponentSprite::SetScaling(int index, float scalingX, float scalingY) {
  if (!SpriteExists(index)) {
    return;
  }
  RenderSpriteBatch::Sprite *sprite = mRenderableSprites[index].get();
  sprite->scaleX = scalingX;
  sprite->scaleY = scalingY;
}
void ComponentSprite::SetScalingX(int index, float scalingX) {
  if (!SpriteExists(index)) {
    return;
  }
  mRenderableSprites[index].get()->scaleX = scalingX;
}
void ComponentSprite::SetScalingY(int index, float scalingY) {
  if (!SpriteExists(index)) {
    return;
  }
  mRenderableSprites[index].get()->scaleY = scalingY;
}

void ComponentSprite::SetAnimation(int index,
                                   const std::string &animationName) {
  if (!SpriteExists(index)) {
    return;
  }
  mAnimationData[index].SetAnimation(animationName);
}

void ComponentSprite::SetAnimationSpeed(int index, float speed) {
  if (!SpriteExists(index)) {
    return;
  }
  mAnimationData[index].animationSpeed = speed;
}

float ComponentSprite::DefaultAnimationSpeed(int index) {
  if (!SpriteExists(index)) {
    return 0.0f;
  }
  return mAnimationData[index].defaultAnimationSpeed;
}

void ComponentSprite::SetImageIndex(int index, int imageIndex) {
  if (!SpriteExists(index)) {
    return;
  }
  mAnimationData[index].SetImageIndex(imageIndex);
}

bool ComponentSprite::RenderSprite(int index, bool render) {
  if (!SpriteExists(index)) {
    return false;
  }
  mRenderableSprites[index].get()->isActive = render;
  return true;
}

bool ComponentSprite::AnimateSprite(int index, bool animate) {
  if (!SpriteExists(index)) {
    return false;
  }
  mAnimationData[index].animate = animate;
  return true;
}

void ComponentSprite::SetOffset(int index, float x, float y) {
  if (!SpriteExists(index)) {
    return;
  }

  mRenderableSprites[index]->offset.x = x;
  mRenderableSprites[index]->offset.y = y;
}

int ComponentSprite::AddSprite(const RSC_Sprite *sprite, const MAP_DEPTH &depth,
                               float x, float y) {
  Vec2 offset(x, y);
  AnimationData data(sprite->GetAnimationMapPointer());
  auto textureName = sprite->GetTextureName();
  const RSC_Texture *texture =
      K_TextureMan.GetLoadItem(textureName, textureName);
  if (texture == NULL) {
    std::stringstream ss;
    ss << "[C++] ComponentSprite::AddSprite couldn't find Texture named "
       << sprite->GetTextureName();
    LOG_INFO(ss.str());
    return -1;
  }

  mSprites.push_back(sprite);
  mAnimationData.push_back(data);
  mRenderableSprites.push_back(std::make_unique<RenderSpriteBatch::Sprite>(
      rm, sprite->GetTextureName(), texture->GetWidth(), texture->GetHeight(),
      depth, offset));

  CalculateVerticies(mNumberOfLoadedSprites);

  int idToReturn = mNumberOfLoadedSprites;  // return handle to this sprite
  mNumberOfLoadedSprites += 1;
  return idToReturn;
}

void ComponentSprite::HandleEvent(const Event *event) {}

//////////////////////////
// ComponentSpriteManager//
//////////////////////////

ComponentSpriteManager::ComponentSpriteManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e) {}

std::unique_ptr<ComponentSprite> ComponentSpriteManager::ConstructComponent(
    EID id, ComponentSprite *parent) {
  auto sprite = std::make_unique<ComponentSprite>(
      id, (ComponentPosition *)dependencyPosition->GetComponent(id),
      dependencyRenderManager, this);

  return std::move(sprite);
}
void ComponentSpriteManager::SetDependencies(ComponentPositionManager *pos,
                                             RenderManager *rm) {
  dependencyPosition = pos;
  dependencyRenderManager = rm;
}
