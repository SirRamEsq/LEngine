#include "RSC_Sprite.h"

#include "ResourceLoading.h"
#include "../Defines.h"

#include "../Kernel.h"

#include <stdio.h>
#include <string.h>
//////////////
//LAnimation//
//////////////

void LAnimation::CalculateUV(int textureWidth, int textureHeight){
	if ( (textureWidth <= 0 ) or (textureHeight <= 0 ) ){return;}	

	UVCoords.clear();
	for(auto i = images.begin(); i != images.end(); i++){
		float leftUV = ((float)(*i).GetLeft()) / textureWidth; 
		float topUV = ((float)(*i).GetTop()) / textureHeight;
		float rightUV = ((float)(*i).GetRight())/ textureWidth;
		float bottomUV = ((float)(*i).GetBottom()) / textureHeight;

		UVCoords.push_back(std::pair<Coord2df,Coord2df>(
			Coord2df(
				leftUV,
				topUV	
			),
			Coord2df(
				rightUV,
				bottomUV
			)
		));
	}
	isUVCalculated = true;
}

std::pair<Coord2df,Coord2df> LAnimation::GetUVRandom(int index) const{
	if((ValidateIndex(index)) and (isUVCalculated)){
		//Get float between 0.0 and 1.0
		float normalizedRandom = ((float) rand() / (RAND_MAX));
		float leftRightDif = GetUVRight(index) - GetUVLeft(index);
		float topBottomDif = GetUVBottom(index) - GetUVTop(index);
		float rightUV, leftUV, topUV, bottomUV;

		//right side
		//get between frame left and frame right
		rightUV = GetUVLeft(index) + (leftRightDif * normalizedRandom);

		//left side
		//Get between frame left and rightUV
		normalizedRandom = ((float) rand() / (RAND_MAX));
		leftUV = rightUV - ((rightUV - GetUVLeft(index))*normalizedRandom);

		//bottom side
		//get between frame top and frame bottom
		normalizedRandom = ((float) rand() / (RAND_MAX));
		bottomUV = GetUVTop(index) + ( topBottomDif * normalizedRandom );

		//top side
		//Get between frame top and bottomUV
		normalizedRandom = ((float) rand() / (RAND_MAX));
		topUV = bottomUV - ((bottomUV - GetUVTop(index))*normalizedRandom);
				
		return std::pair<Coord2df,Coord2df>(Coord2df(leftUV,topUV), Coord2df(rightUV,bottomUV));
	}
	return std::pair<Coord2df,Coord2df>(Coord2df(0,0), Coord2df(0,0));
}

bool LAnimation::ValidateIndex(int index) const {
	return ((index>=0)and(index<images.size()));
}

void LAnimation::SetColorKey(int image, unsigned int r, unsigned int g, unsigned int b){
//    images[image]->SetColorKey(r,g,b);
}

void LAnimation::AppendImage(const Rect& img){
    images.push_back(Rect(img));
}

int LAnimation::GetWidth(int index)const {
	if(ValidateIndex(index)){
    	return images[index].w;	
	}
	return 0;
}

int LAnimation::GetHeight(int index)const {
	if(ValidateIndex(index)){
    	return images[index].h;
	}
	return 0;
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
	isUVCalculated = false;
}

const Rect& LAnimation::GetRectAtIndex(int imageIndex)const {
	if(ValidateIndex(imageIndex)){
    	return images[imageIndex];
	}
	throw LEngineException("Invalid index for animation");
}

float LAnimation::GetUVTop(int index) const{
	if((ValidateIndex(index)) and (isUVCalculated)){
		return (std::get<0>(UVCoords[index])).y; 
	}
	return 0;
}

float LAnimation::GetUVRight(int index) const{
	if((ValidateIndex(index)) and (isUVCalculated)){
		return (std::get<1>(UVCoords[index])).x; 
	}
	return 1;
}

float LAnimation::GetUVBottom(int index) const{
	if((ValidateIndex(index)) and (isUVCalculated)){
		return (std::get<1>(UVCoords[index])).y; 
	}
	return 1;
}

float LAnimation::GetUVLeft(int index) const{
	if((ValidateIndex(index)) and (isUVCalculated)){
		return (std::get<0>(UVCoords[index])).x; 
	}
	return 0;
}

///////////
//RSC_Sprite//
///////////

RSC_Sprite::RSC_Sprite(const std::string& sname) : spriteName(sname){
    origin=L_ORIGIN_CENTER;
}

RSC_Sprite::~RSC_Sprite(){
}

void RSC_Sprite::SetColorKey(const std::string& aniName, unsigned int image, unsigned int r, unsigned int g, unsigned int b){
    animations.find(aniName)->second.SetColorKey(image, r,g,b);
}

const LAnimation* RSC_Sprite::GetAnimation(const std::string& aniName) const {
    auto i=animations.find(aniName);
    if(i==animations.end()){
		std::stringstream ss;
		ss << "Sprite '" << spriteName << "' Couldn't get animation ' " << aniName << "'";
		throw LEngineException(ss.str());
    }
    return &(i->second);
}


void RSC_Sprite::SetOrigin(LOrigin o){
    origin=o;
}

bool RSC_Sprite::LoadFromXML(const char* dat, unsigned int fsize){
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


    xml_node<>* animationNode=node->first_node(); //point to the first child of <sprite>

    std::string animationTagName = "";
    for(; animationNode!=0; animationNode=animationNode->next_sibling()){ //Get all animations
        animationTagName = animationNode->name();
		LAnimation* newAnimation = NULL;
        if(animationTagName == "animation"){
            newAnimation = LoadAnimation(animationNode);
        }
        else if(animationTagName == "animationSequence"){
            newAnimation = LoadAnimationSequence(animationNode);
        }
        else{
            std::stringstream ss;
            ss << "[C++] RSC_Sprite::LoadFromXML, couldn't load xml tag named " << animationNode->name() << " For sprite " << spriteName;
            LOG_INFO(ss.str());
            continue;
        }

		if(newAnimation != NULL){
			auto texture = K_TextureMan.GetLoadItem(mTextureName, mTextureName);
			if(texture != NULL){
				newAnimation->CalculateUV(texture->GetWidth(), texture->GetHeight());
			}
		}
		
    }
    return true;
}

LAnimation* RSC_Sprite::LoadAnimation(rapidxml::xml_node<>* animationNode){
    using namespace rapidxml;

    LAnimation* animation;
    xml_node<>* imageNode;
    Rect rect;
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
	return animation;
}
LAnimation* RSC_Sprite::LoadAnimationSequence(rapidxml::xml_node<>* animationNode){
    using namespace rapidxml;

    LAnimation* animation;
    Rect startRect;
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

    Rect rect(startRect);
    for(int i = 1; i <= imgs; i++){
        animation->AppendImage(rect);

        rect.x += rect.w;
    }
	return animation;
}

std::unique_ptr<RSC_Sprite> RSC_Sprite::LoadResource(const std::string& fname){
    std::unique_ptr<RSC_Sprite> sprite = NULL;
    try{
        std::string fullPath = "Resources/Sprites/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        sprite = std::make_unique<RSC_Sprite>(fname);
        if(sprite->LoadFromXML(data.get()->GetData(), data.get()->length)==false){
            LOG_INFO("Couldn't parse XML Sprite Data for sprite " + fname);
            return NULL;
        }
    }
    catch(LEngineFileException e){
        LOG_INFO(e.what());
    }

    return sprite;
}
