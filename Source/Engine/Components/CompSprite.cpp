#include "CompSprite.h"
#include "../Kernel.h"

unsigned int ComponentSprite::MAX_SPRITES_DEFAULT = 8;

AnimationData::AnimationData(const RSC_Sprite *spr)
    : animations(spr->GetAnimationMapPointer()) {
  animate = true;
  playOnce = false;
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
    if (playOnce) {
      animate = false;
      currentTime = maxTime;
      playOnceCallback();
    } else {
      currentTime -= maxTime;
    }
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

bool AnimationData::SetAnimationPlayOnce(const std::string &aniName,
                                         Callback callback) {
  auto returnValue = SetAnimation(aniName);
  if (returnValue) {
    playOnce = true;
    playOnceCallback = callback;
  }
  return returnValue;
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
  animate = true;
  playOnce = false;

  return true;
}

ComponentSprite::ComponentSprite(EID id, ComponentPosition *pos,
                                 RenderManager *r,
                                 ComponentSpriteManager *manager)
    : BaseComponent(id, manager) {
  SetMaxSprites(MAX_SPRITES_DEFAULT);
  rm = r;
  mEntityID = id;
  myPos = pos;
}

ComponentSprite::~ComponentSprite() {}

void ComponentSprite::Update() {
  Vec2 pos = myPos->GetPositionWorld();

  for (auto i = mSprites.begin(); i != mSprites.end(); i++) {
    auto renderSprite = &(i->mRenderableSprite);
    if (!renderSprite->isActive) {
      continue;
    }

    auto animation = &i->mAnimation;
    auto spriteRSC = i->mSpriteRSC;

    animation->Update();

    auto ani = spriteRSC->GetAnimation(animation->GetAnimation());
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

    // Translate position back from the origin offset
    renderSprite->data.translate.x = xPos - renderSprite->data.vertexOrigin1.x;
    renderSprite->data.translate.y = yPos - renderSprite->data.vertexOrigin1.y;
  }
}

Sprite::Sprite(RenderManager *rm, const RSC_Sprite *sprite, MAP_DEPTH depth)
    : mAnimation(sprite),
      mRenderableSprite(rm, sprite, depth, Vec2(0, 0)),
      mSpriteRSC(sprite) {
  auto textureName = sprite->GetTextureName();
  const RSC_Texture *texture = K_TextureMan.GetLoadItem(textureName);

  if (texture == NULL) {
    std::stringstream ss;
    ss << "Couldn't find Texture named " << textureName;
    LOG_INFO(ss.str());
  }

  CalculateVerticies();
}

void Sprite::SetDepth(MAP_DEPTH depth) { mRenderableSprite.SetDepth(depth); }
MAP_DEPTH Sprite::GetDepth() const { return mRenderableSprite.GetDepth(); }

void Sprite::AnimationPlayOnce(const std::string &animationName,
                               luabridge::LuaRef callback) {
  auto callbackLambda = [callback]() { callback(); };
  mAnimation.SetAnimationPlayOnce(animationName, callbackLambda);
}

void Sprite::SetAlpha(float a) { mRenderableSprite.SetAlpha(a); }
float Sprite::GetAlpha() const { return mRenderableSprite.GetAlpha(); }
void Sprite::SetColor(Vec4 c) { mRenderableSprite.SetColor(c); }
Vec4 Sprite::GetColor() const { return mRenderableSprite.GetColor(); }
void Sprite::CalculateVerticies() {
  LOrigin origin = mSpriteRSC->GetOrigin();
  float spriteWidth = mSpriteRSC->GetWidth();
  float spriteHeight = mSpriteRSC->GetHeight();

  int spriteWidthHalf = spriteWidth / 2;
  int spriteHeightHalf = spriteHeight / 2;

  // laconic access
  auto rSprite = &mRenderableSprite;

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

void Sprite::SetRotation(float rotation) {
  mRenderableSprite.rotation = rotation;
}
void Sprite::SetScaling(float scalingX, float scalingY) {
  mRenderableSprite.scaleX = scalingX;
  mRenderableSprite.scaleY = scalingY;
}
void Sprite::SetScalingX(float scalingX) {
  mRenderableSprite.scaleX = scalingX;
}
void Sprite::SetScalingY(float scalingY) {
  mRenderableSprite.scaleY = scalingY;
}

void Sprite::SetAnimation(const std::string &animationName) {
  mAnimation.SetAnimation(animationName);
}

void Sprite::SetAnimationSpeed(float speed) {
  mAnimation.animationSpeed = speed;
}

float Sprite::GetAnimationSpeed() { return mAnimation.animationSpeed; }

float Sprite::DefaultAnimationSpeed() {
  return mAnimation.defaultAnimationSpeed;
}

void Sprite::SetImageIndex(int imageIndex) {
  mAnimation.SetImageIndex(imageIndex);
}

int Sprite::GetImageIndex() { return mAnimation.GetImageIndex(); }

void Sprite::Render(bool render) { mRenderableSprite.isActive = render; }

void Sprite::SetOffset(float x, float y) {
  mRenderableSprite.offset.x = x;
  mRenderableSprite.offset.y = y;
}

void ComponentSprite::SetMaxSprites(unsigned int spriteCount) {
  mMaxSprites = spriteCount;
  mSprites.reserve(sizeof(Sprite) * spriteCount);
}

Sprite *ComponentSprite::AddSprite(const RSC_Sprite *sprite) {
  if (mSprites.size() >= mMaxSprites) {
    std::stringstream ss;
    ss << "Will not create sprite, number of sprites is equal to max "
       << mMaxSprites << "'" << std::endl
       << "In Script '" << Kernel::GetNameFromEID(mEntityID) << "'";
    LOG_ERROR(ss.str());
    return NULL;
  }

  if (sprite == NULL) {
    return NULL;
  }

  mSprites.emplace(mSprites.end(), rm, sprite, mDefaultDepth);
  return &mSprites.back();
}

void ComponentSprite::HandleEvent(const Event *event) {}

void ComponentSprite::SetDepth(MAP_DEPTH depth) { mDefaultDepth = depth; }
MAP_DEPTH ComponentSprite::GetDepth() const { return mDefaultDepth; }

//////////////////////////
// ComponentSpriteManager//
//////////////////////////

ComponentSpriteManager::ComponentSpriteManager(EventDispatcher *e)
    : BaseComponentManager_Impl(e) {}

std::unique_ptr<ComponentSprite> ComponentSpriteManager::ConstructComponent(
    EID id, ComponentSprite *parent) {
  if (dependencyPosition->GetComponent(id) == NULL) {
    dependencyPosition->AddComponent(id);
  }
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

void ComponentSpriteManager::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")

      .beginClass<Sprite>("Sprite")
      .addFunction("SetAnimation", &Sprite::SetAnimation)
      .addFunction("SetAnimationSpeed", &Sprite::SetAnimationSpeed)
      .addFunction("AnimationPlayOnce", &Sprite::AnimationPlayOnce)
      .addFunction("GetAnimationSpeed", &Sprite::GetAnimationSpeed)
      .addFunction("DefaultAnimationSpeed", &Sprite::DefaultAnimationSpeed)
      .addFunction("SetImage", &Sprite::SetImageIndex)
      .addFunction("GetImage", &Sprite::GetImageIndex)
      .addFunction("SetDepth", &Sprite::SetDepth)
      .addFunction("GetDepth", &Sprite::GetDepth)
      .addFunction("SetAlpha", &Sprite::SetAlpha)
      .addFunction("GetAlpha", &Sprite::GetAlpha)
      .addFunction("SetColor", &Sprite::SetColor)
      .addFunction("GetColor", &Sprite::GetColor)

      .addFunction("SetRotation", &Sprite::SetRotation)
      .addFunction("SetScaling", &Sprite::SetScaling)
      .addFunction("SetScalingX", &Sprite::SetScalingX)
      .addFunction("SetScalingY", &Sprite::SetScalingY)

      .addFunction("SetOffset", &Sprite::SetOffset)

      .addFunction("Render", &Sprite::Render)
      .endClass()

      .beginClass<ComponentSprite>("ComponentSprite")
      .addFunction("AddSprite", &ComponentSprite::AddSprite)
      .addFunction("SetDepth", &ComponentSprite::SetDepth)
      .addFunction("GetDepth", &ComponentSprite::GetDepth)
      .endClass()

      .beginClass<ComponentSpriteManager>("ComponentSpriteManager")
      .endClass()

      .endNamespace();
}
