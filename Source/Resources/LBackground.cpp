#include "LBackground.h"
#include "../Defines.h"
#include "LSprite.h"
#include "../Kernel.h"

LBackground::LBackground(std::string spr, std::string ani, int xx, int yy, bool xrpt, bool yrpt){
    x=xx;
    y=yy;
    spriteName=spr;
    animationName=ani;
    animationSpeed=1.0f;

    flags=0;
    paralaxSpeed=0;
    if(xrpt){flags=(flags|BG_REPEATX);}
    if(yrpt){flags=(flags|BG_REPEATY);}

    const LSprite* sprite=K_SpriteMan.GetItem(spr);
    if(sprite!=NULL){
        //width=sprite->GetWidth();
        //height=sprite->GetHeight();
    }
}

LBackground::LBackground(std::string spr, std::string ani, int xx, int yy, int f, double paralax, double speed){
    x=xx;
    y=yy;
    spriteName=spr;
    animationName=ani;
    animationSpeed=speed;
    paralaxSpeed=paralax;
    flags=f;

    const LSprite* sprite=K_SpriteMan.GetItem(spr);
    if(sprite!=NULL){
        ErrorLog::WriteToFile(spr, ani);
        //width=sprite->GetWidth();
        //height=sprite->GetHeight();
    }
}

LBackground::LBackground(){
    x=0;
    y=0;
    spriteName=BLANK_SPRITE_NAME;
    animationName=BLANK_ANIMATION_NAME;
    animationSpeed=0;
    paralaxSpeed=1;
    flags=0;
    width=0;
    height=0;
}

LBackground& LBackground::operator =(LBackground& bg){
    if (this==&bg) {return *this;}

    x=bg.x;
    y=bg.y;
    spriteName=bg.spriteName;
    animationName=bg.animationName;
    animationSpeed=bg.animationSpeed;
    paralaxSpeed=bg.paralaxSpeed;
    flags=bg.flags;
    width=bg.width;
    height=bg.height;

    return *this;
}
