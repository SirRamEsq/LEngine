#ifndef L_SPRITECOMP
#define L_SPRITECOMP

#include <memory>

#include "../BaseComponent.h"
#include "../Event.h"
#include "CompPosition.h"
#include "../RenderManager.h"
#include "../Resources/LSprite.h"

class ComponentSpriteManager;

struct AnimationData{
    typedef const std::map<std::string, LAnimation> TAnimationMap;
    AnimationData(const TAnimationMap* aniMap);

    void Update();
    bool SetAnimation(const std::string& aniName);
    bool SetImageIndex(const int& imageIndex);
    const std::string&  GetAnimation(){return currentAnimation;}
    const int&          GetImageIndex(){return currentImageIndexInt;}

    bool    animate;
    float   animationSpeed;

    const TAnimationMap*  animations;

    private:
        int         currentImageIndexInt;
        float       currentImageIndex;
        int         maxFrames;
        std::string currentAnimation;
};

class ComponentSprite : public BaseComponent{
    friend class ComponentSpriteManager;

    public:
        ComponentSprite (EID id, ComponentPosition* pos, const std::string& logFile);
        ~ComponentSprite();

        void    Update();
        void    HandleEvent(const Event* event);

        //returns sprite index
        int     AddSprite   (const LSprite* sprite, const MAP_DEPTH& depth, const float& x=0.0f, const float& y=0.0f);

        bool    SetAnimation        (const int& index, const std::string& animationName);
        bool    SetAnimationSpeed   (const int& index, const float& speed);
        bool    SetImageIndex       (const int& index, const int& imageIndex);

        //Set whether a given sprite is rendered
        bool    RenderSprite    (const int& index, const bool& render);
        //Set whether a given sprite is animated
        bool    AnimateSprite   (const int& index, const bool& animate);
        //Set rotation of sprite
        void    SetRotation     (const int& index, const float& rotation);
        //Set Scaling of Sprite
        void    SetScaling     (const int& index, const float& scalingX, const float& scalingY);
        void    SetScalingX    (const int& index, const float& scalingX);
        void    SetScalingY    (const int& index, const float& scalingY);

        bool    SetOffset      (const int& index, const float& x, const float& y);

        bool    SpriteExists    (const int& index);

        void    CalculateVerticies(const int& index);

    protected:
        ComponentPosition* myPos;

    private:
        int mNumberOfLoadedSprites;
        /*
        Cannot delete sprites from component after adding them in, it would mess up the indexing system;\
        Instead of deleting, just turn off rendering
        may want to send actual data struct to lua instead of an integer
        */

        std::vector<AnimationData>      mAnimationData;     //Used to determine what part of the sprite to use
        std::vector<const LSprite*>           mSprites;
        std::vector< std::unique_ptr<RenderableSprite> >   mRenderableSprites; //class owns renderableSprites, which auto adds and deletes itself to the appropriate sprite batch when instantiated and deleted
};

class ComponentSpriteManager : public BaseComponentManager{
    public:
        ComponentSpriteManager();

        void AddComponent(EID id);
        void HandleEvent(const Event* event);
        void Update();
};

/*
=========================================================
===Quick Note about LuaBridge and Multiple Inheritence===
=========================================================
After trying to expose ComponentSprite to luaBridge by declaring it as a derived class of
RenderableObject, I would encounter an inexplicable segfault everytime I tried to run certain
RenderableObject functions. The functions that didn't crash behaved very strangely

As it turns out I had declared ComponentSprite's inheritence as;
class ComponentSprite : public BaseComponent, public RenderableObject

Because I declared RenderableObject SECOND, LuaBridge couldn't make sense of it
Changing the order of inheritence fixed the problem completely

*/

#endif
