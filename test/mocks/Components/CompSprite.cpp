#include "../../../Source/Components/CompSprite.h"
#include "../../../Source/Kernel.h"

#ifdef TEST_ComponentScript_MOCK

AnimationData::AnimationData(const TAnimationMap* aniMap)
    : animations(aniMap){

    animate=true;
    currentImageIndex=0;
    currentImageIndexInt=0;
    animationSpeed=0.0f;
    SetAnimation(animations->begin()->first);
}


void AnimationData::Update(){
    if(!animate){return;}

    currentImageIndex+=animationSpeed;
    if(floor(currentImageIndex+0.5f)>=maxFrames){
        currentImageIndex-=maxFrames;
    }
    currentImageIndexInt= floor(currentImageIndex+0.5f);
}

bool AnimationData::SetImageIndex(const int& imageIndex){
    if(imageIndex>=maxFrames){return false;}

    currentImageIndex=imageIndex;
    currentImageIndexInt=imageIndex;

    return true;
}

bool AnimationData::SetAnimation(const std::string& aniName){
    if(currentAnimation==aniName){return true;}
    auto animation=animations->find(aniName);
    if(animation==animations->end()){return false;}

    currentAnimation =      aniName;
    currentImageIndex =     0.0f;
    currentImageIndexInt =  0;
    maxFrames=              animation->second.NumberOfImages();
    animationSpeed=         animation->second.GetSpeed();

    return true;
}

ComponentSprite::ComponentSprite(EID id, const std::string& logName) : BaseComponent(id, logName){
    mEntityID=id;
    mNumberOfLoadedSprites=0;
}

ComponentSprite::~ComponentSprite(){
}


void ComponentSprite::Update(){

}

void ComponentSprite::CalculateVerticies(const int& index){
    LOrigin origin=mSprites[index]->GetOrigin();
    float spriteWidth=mSprites[index]->GetWidth();
    float spriteHeight=mSprites[index]->GetHeight();
    //Pixel '0' counts as a pixel for the purpose of our calculations
    spriteHeight -=1;
    spriteWidth  -=1;

    float spriteWidthHalf=spriteWidth/2;
    float spriteHeightHalf=spriteHeight/2;
/*
    RenderableSprite* rSprite=mRenderableSprites[index].get();

    switch(origin){
        case L_ORIGIN_CENTER:
            rSprite->data.vertexOrigin1.x = spriteWidthHalf*-1;     rSprite->data.vertexOrigin1.y = spriteHeightHalf*-1;  //Top Left
            rSprite->data.vertexOrigin2.x = spriteWidthHalf;        rSprite->data.vertexOrigin2.y = spriteHeightHalf*-1;  //Top Right
            rSprite->data.vertexOrigin3.x = spriteWidthHalf;        rSprite->data.vertexOrigin3.y = spriteHeightHalf;     //Bottom Right
            rSprite->data.vertexOrigin4.x = spriteWidthHalf*-1;     rSprite->data.vertexOrigin4.y = spriteHeightHalf;     //Bottom Left
            break;

        case L_ORIGIN_TOP_LEFT:
            rSprite->data.vertexOrigin1.x = 0;                      rSprite->data.vertexOrigin1.y = 0;
            rSprite->data.vertexOrigin2.x = spriteWidth;            rSprite->data.vertexOrigin2.y = 0;
            rSprite->data.vertexOrigin3.x = spriteWidth;            rSprite->data.vertexOrigin3.y = spriteHeight;
            rSprite->data.vertexOrigin4.x = 0;                      rSprite->data.vertexOrigin4.y = spriteHeight;
            break;

        case L_ORIGIN_TOP_RIGHT:
            rSprite->data.vertexOrigin1.x = spriteWidth;            rSprite->data.vertexOrigin1.y = 0;
            rSprite->data.vertexOrigin2.x = 0;                      rSprite->data.vertexOrigin2.y = 0;
            rSprite->data.vertexOrigin3.x = 0;                      rSprite->data.vertexOrigin3.y = spriteHeight;
            rSprite->data.vertexOrigin4.x = spriteWidth;            rSprite->data.vertexOrigin4.y = spriteHeight;
            break;

        case L_ORIGIN_BOTTOM_RIGHT:
            rSprite->data.vertexOrigin1.x = spriteWidth;            rSprite->data.vertexOrigin1.y = spriteHeight;
            rSprite->data.vertexOrigin2.x = 0;                      rSprite->data.vertexOrigin2.y = spriteHeight;
            rSprite->data.vertexOrigin3.x = 0;                      rSprite->data.vertexOrigin3.y = 0;
            rSprite->data.vertexOrigin4.x = spriteWidth;            rSprite->data.vertexOrigin4.y = 0;
            break;

        case L_ORIGIN_BOTTOM_LEFT:
            rSprite->data.vertexOrigin1.x = 0;                      rSprite->data.vertexOrigin1.y = spriteHeight;
            rSprite->data.vertexOrigin2.x = spriteWidth;            rSprite->data.vertexOrigin2.y = spriteHeight;
            rSprite->data.vertexOrigin3.x = spriteWidth;            rSprite->data.vertexOrigin3.y = 0;
            rSprite->data.vertexOrigin4.x = 0;                      rSprite->data.vertexOrigin4.y = 0;
            break;
        }
        */
}

bool ComponentSprite::SpriteExists(const int& index){
    return (index<mNumberOfLoadedSprites);
}

void ComponentSprite::SetRotation(const int& index, const float& rotation){
    if(!SpriteExists(index)){return;}
//    mRenderableSprites[index].get()->rotation=rotation;
}
void ComponentSprite::SetScaling(const int& index, const float& scalingX, const float& scalingY){
    if(!SpriteExists(index)){return;}
}
void ComponentSprite::SetScalingX(const int& index, const float& scalingX){
    if(!SpriteExists(index)){return;}
//    mRenderableSprites[index].get()->scaleX=scalingX;
}
void ComponentSprite::SetScalingY(const int& index, const float& scalingY){
    if(!SpriteExists(index)){return;}
 //   mRenderableSprites[index].get()->scaleY=scalingY;
}


bool ComponentSprite::SetAnimation(const int& index, const std::string& animationName){
    if(index>=mNumberOfLoadedSprites){return false;}
    return mAnimationData[index].SetAnimation(animationName);
}

bool ComponentSprite::SetAnimationSpeed(const int& index, const float& speed){
    if(index>=mNumberOfLoadedSprites){return false;}
    mAnimationData[index].animationSpeed=speed;
    return true;
}

bool ComponentSprite::SetImageIndex(const int& index, const int& imageIndex){
    if(index>=mNumberOfLoadedSprites){return false;}
    return mAnimationData[index].SetImageIndex(imageIndex);
}

bool ComponentSprite::RenderSprite(const int& index, const bool& render){
    if(index>=mNumberOfLoadedSprites){return false;}
//    mRenderableSprites[index].get()->isActive=render;
    return true;
}

bool ComponentSprite::AnimateSprite(const int& index, const bool& animate){
    if(index>=mNumberOfLoadedSprites){return false;}
    mAnimationData[index].animate=animate;
    return true;
}

bool ComponentSprite::SetOffset(const int& index, const float& x, const float& y){
    if(index>=mNumberOfLoadedSprites){return false;}

//    mRenderableSprites[index]->offset.x = x;
//   mRenderableSprites[index]->offset.y = y;

    return true;
}

int ComponentSprite::AddSprite(const LSprite* sprite, const MAP_DEPTH& depth, const float& x, const float& y){
    Vec2 offset(x,y);
    AnimationData data(sprite->GetAnimationMapPointer());
    const LTexture* texture=K_TextureMan.GetItem(sprite->GetTextureName());
    if(texture==NULL){
        std::stringstream ss;
        ss << "[C++] ComponentSprite::AddSprite couldn't find Texture named " << sprite->GetTextureName();
        ErrorLog::WriteToFile(ss.str(), logFileName);
        return -1;
    }

    mSprites.            push_back(sprite);
    mAnimationData.      push_back(data);
    //mRenderableSprites.  push_back(std::unique_ptr<RenderableSprite>(new RenderableSprite (sprite->GetTextureName(), texture->GetWidth(), texture->GetHeight(), depth, offset)));

    CalculateVerticies(mNumberOfLoadedSprites);

    int idToReturn=mNumberOfLoadedSprites; //return handle to this sprite
    mNumberOfLoadedSprites+=1;
    return idToReturn;
}

void ComponentSprite::HandleEvent(const Event* event){}

//////////////////////////
//ComponentSpriteManager//
//////////////////////////

ComponentSpriteManager::ComponentSpriteManager() : BaseComponentManager("LOG_COMP_SPRITE"){

}

void ComponentSpriteManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentSprite* sprite=new ComponentSprite(id, logFileName);
    componentList[id]=sprite;
}

void ComponentSpriteManager::HandleEvent(const Event* event){

}

void ComponentSpriteManager::Update(){
    for(auto i=componentList.begin(); i!=componentList.end(); i++){
       i->second->Update();
    }
}

#endif
