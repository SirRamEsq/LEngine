#include "LSprite.h"

#include "../Kernel.h"

//////////////
//LAnimation//
//////////////

void LAnimation::SetColorKey(int image, unsigned int r, unsigned int g, unsigned int b){
//    images[image]->SetColorKey(r,g,b);
}

void LAnimation::AppendImage(const CRect& img){
    images.push_back(CRect(img));
}

int LAnimation::GetWidth(int index)const {
    return images[index].w;
}

int LAnimation::GetHeight(int index)const {
    return images[index].h;
}

LAnimation::~LAnimation(){
    DeleteImages();
}

void LAnimation::DeleteImages(){
    images.clear();
}

LAnimation::LAnimation(const double& spd, AnimationLoadTag t) : loadTag(t){
    defaultSpeed=spd;
    currentImage=0;
}

const CRect& LAnimation::GetCRectAtIndex(const int& imageIndex)const {
    return images[imageIndex];
}

///////////
//LSprite//
///////////

LSprite::LSprite(const std::string& sname) : spriteName(sname){
    origin=L_ORIGIN_CENTER;
}

LSprite::~LSprite(){
}

void LSprite::SetColorKey(const std::string& aniName, unsigned int image, unsigned int r, unsigned int g, unsigned int b){
    animations.find(aniName)->second.SetColorKey(image, r,g,b);
}

const LAnimation* LSprite::GetAnimation(const std::string& aniName) const {
    auto i=animations.find(aniName);
    if(i==animations.end()){
        return NULL;
    }
    return &(i->second);
}


void LSprite::SetOrigin(LOrigin o){
    origin=o;
}

bool LSprite::LoadFromXML(const char* dat, unsigned int fsize){
    std::string XML=std::string(dat,fsize);

    using namespace rapidxml;
    xml_document<> doc;    // character type defaults to char
    doc.parse<0>((char*)(XML.c_str()));

    //Find Specific Node
    xml_node<>* node = doc.first_node("sprite");

        xml_attribute<>* attribute;
        attribute = node->first_attribute("origin");
        if(attribute!=NULL){
            std::string originString=attribute->value();
            if      (originString=="center")      {origin=L_ORIGIN_CENTER;}
            else if (originString=="topleft")     {origin=L_ORIGIN_TOP_LEFT;}
            else if (originString=="topright")    {origin=L_ORIGIN_TOP_RIGHT;}
            else if (originString=="bottomleft")  {origin=L_ORIGIN_BOTTOM_LEFT;}
            else if (originString=="bottomright") {origin=L_ORIGIN_BOTTOM_RIGHT;}
            else                                  {origin=L_ORIGIN_CENTER;}
        }

        attribute = node->first_attribute("width");
        if(attribute!=NULL){width=strtol(attribute->value(), NULL, 10);} //Needs width
        else {return false;}

        attribute = node->first_attribute("height");
        if(attribute!=NULL){height=strtol(attribute->value(), NULL, 10);} //Needs height
        else {return false;}

        std::stringstream transparentColor;
        std::stringstream transparentColorTemp;
        std::string red, green, blue;
        char c;
        attribute = node->first_attribute("transparentColor");
        if(attribute!=NULL){
            int colorIndex=0;
            int characterCount=0;
            const char* matches = "1234567890aAbBcCdDeEfF";//Only notice the character if it is A-F or 0-9
            transparentColor << attribute->value();

            while(transparentColor.get(c)){
                if (strchr(matches, c) != NULL) {
                    transparentColorTemp << c; //Add character to stringstream
                    characterCount++;
                }
                if(characterCount>=2){
                    if     (colorIndex==0){transparentColorTemp >> std::hex >> transparentColorRed;   }
                    else if(colorIndex==1){transparentColorTemp >> std::hex >> transparentColorGreen; }
                    else if(colorIndex==2){transparentColorTemp >> std::hex >> transparentColorBlue;  }//Wrong
                    else                  {break;} //Colors already have been read; exit loop
                    colorIndex++;
                    characterCount=0;
                    //Clear string stream
                    transparentColorTemp.str(std::string());
                    transparentColorTemp.clear();
                }
            }
        }

        attribute = node->first_attribute("textureName");
        if(attribute!=NULL){mTextureName=attribute->value();}
        K_TextureMan.LoadItem(mTextureName, mTextureName);


    xml_node<>* animationNode=node->first_node(); //point to the first child of <sprite>

    std::string animationTagName = "";
    for(; animationNode!=0; animationNode=animationNode->next_sibling()){ //Get all animations
        animationTagName = animationNode->name();
        if(animationTagName == "animation"){
            LoadAnimation(animationNode);
        }
        else if(animationTagName == "animationSequence"){
            LoadAnimationSequence(animationNode);
        }
        else{
            std::stringstream ss;
            ss << "[C++] LSprite::LoadFromXML, couldn't load xml tag named " << animationNode->name() << " For sprite " << spriteName;
            ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
            continue;
        }
    }
    return true;
}

void LSprite::LoadAnimation(rapidxml::xml_node<>* animationNode){
    using namespace rapidxml;

    LAnimation* animation;
    xml_node<>* imageNode;
    CRect rect;
    std::string animationName, speed;
    xml_attribute<>* attribute = animationNode->first_attribute("name");
    if(attribute!=NULL){animationName=attribute->value();}

    attribute = animationNode->first_attribute("speed");
    if(attribute!=NULL){speed=attribute->value();}

    animations.insert(std::make_pair(animationName, LAnimation( (atof(speed.c_str())), LOAD_TAG_ANIMATION)));
    animation=&animations.find(animationName)->second;

    for(imageNode=animationNode->first_node(); imageNode!=0; imageNode=imageNode->next_sibling()){//Get all images
        attribute = imageNode->first_attribute("x");
        if(attribute!=NULL){rect.x=strtol(attribute->value(), NULL, 10);}

        attribute = imageNode->first_attribute("y");
        if(attribute!=NULL){rect.y=strtol(attribute->value(), NULL, 10);}

        attribute = imageNode->first_attribute("w");
        if(attribute!=NULL){rect.w=strtol(attribute->value(), NULL, 10);}

        attribute = imageNode->first_attribute("h");
        if(attribute!=NULL){rect.h=strtol(attribute->value(), NULL, 10);}

        animation->AppendImage(rect);
    }
}
void LSprite::LoadAnimationSequence(rapidxml::xml_node<>* animationNode){
    using namespace rapidxml;

    LAnimation* animation;
    CRect startRect;
    int imgs = 0;
    std::string animationName, speed;
    xml_attribute<>* attribute = animationNode->first_attribute("name");
    if(attribute!=NULL){animationName=attribute->value();}

    attribute = animationNode->first_attribute("speed");
    if(attribute!=NULL){speed=attribute->value();}

    animations.insert(std::make_pair(animationName, LAnimation( (atof(speed.c_str())), LOAD_TAG_ANIMATION_SEQUENCE)));
    animation=&animations.find(animationName)->second;

    //Get parameters from xml node attributes
    attribute = animationNode->first_attribute("startX");
    if(attribute!=NULL){startRect.x=strtol(attribute->value(), NULL, 10);}

    attribute = animationNode->first_attribute("startY");
    if(attribute!=NULL){startRect.y=strtol(attribute->value(), NULL, 10);}

    attribute = animationNode->first_attribute("w");
    if(attribute!=NULL){startRect.w=strtol(attribute->value(), NULL, 10);}

    attribute = animationNode->first_attribute("h");
    if(attribute!=NULL){startRect.h=strtol(attribute->value(), NULL, 10);}

    attribute = animationNode->first_attribute("imgs");
    if(attribute!=NULL){imgs=strtol(attribute->value(), NULL, 10);}

    CRect rect(startRect);
    for(int i = 1; i <= imgs; i++){
        animation->AppendImage(rect);

        rect.x += rect.w;
    }

}
