#include "LTDF.h"
#include "../Kernel.h"
/*
LTDF::LTDF(std::string tdfn, std::string sprn, std::string anin, std::string hmname, double frict, int32_t flgs)
 : tdfName(tdfn), spriteName(sprn), animationName(anin), heightmapName(hmname), friction(frict), flags(flgs){
    sprite = new LSprite("NULL");
    ReloadHeightmap();
}

LTDF::~LTDF(){
    delete sprite;
}

void LTDF::ReloadHeightmap(){
    LHeightmap* hm=K_HMapMan->GetItem(heightmapName);
    if(hm==NULL){ErrorLog::WriteToFile("Height map is NULL: ", heightmapName);}
    hmap=hm;
}

void LTDF::LoadSprite(){
    if(K_SpriteMan.GetItem(spriteName)!=NULL){
//        *sprite=*(K_SpriteMan.GetItem(spriteName));
//        sprite->SetCurrentAnimation(animationName);
    }
    else{ErrorLog::WriteToFile(spriteName, " doesn't exist");}
}

int LTDF::GetHeightAtX(int x){
    if(x>15){x=15;}
    if(x<0){x=0;}
    if(hmap==NULL){return 16;}
    return hmap->heightMapH[x];
}

int LTDF::GetHeightAtY(int y){
    if(y>15){y=15;}
    if(y<0){y=0;}
    if(hmap==NULL){return 16;}
    return hmap->heightMapV[y];
}

int LTDF::GetAngleH(){
    if(hmap==NULL){return 0;}
    return hmap->angleH;
}

int LTDF::GetAngleV(){
    if(hmap==NULL){return 0;}
    return hmap->angleV;
}

LTDF& LTDF::operator =(LTDF& tdf){
    if (this==&tdf) {return *this;}

    friction=tdf.friction;
    flags=tdf.flags;
    tdfName=tdf.tdfName;
    animationName=tdf.animationName;
    spriteName=tdf.spriteName;
    heightmapName=tdf.heightmapName;
    ReloadHeightmap();

    return *this;
}*/
