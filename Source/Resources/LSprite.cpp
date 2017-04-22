#include "LSprite.h"

#include "../Kernel.h"//So that LImage can access the TextureMan
/*
LImage::LImage(const std::string& fileName, const CRect& coord, LOrigin origin)
  : mTexture(NULL), mRectCoord(coord){

    mTexture = K_TextureMan.GetItem(fileName);
    if(mTexture==NULL){
	    K_TextureMan.LoadItem(fileName, fileName);
	    mTexture= K_TextureMan.GetItem(fileName);
	    if(mTexture==NULL){
            ErrorLog::WriteToFile("LImage couldn't Load texture: ", fileName);
	    }
	}

	mRectCoord=coord;
	mOldOrigin=origin;

	Initialize();
}

LImage::LImage(LTexture* texture, const CRect& coord, LOrigin origin){
    mTexture = texture;
    mRectCoord = coord;
    mOldOrigin = origin;

    Initialize();
}

void LImage::Initialize(){
    //Compensate for CRect Width and Height Truncation
    mRectCoord.heightTruncation=false;

    // Get the coordinates of the image in the texture, expressed
    // as a value from 0 to 1.
    mTexTop	 = ((float)mRectCoord.GetTop())/mTexture->GetHeight();
    mTexBottom = ((float)mRectCoord.GetBottom())/mTexture->GetHeight();
    mTexLeft   = ((float)mRectCoord.GetLeft())/mTexture->GetWidth();
    mTexRight  = ((float)mRectCoord.GetRight())/mTexture->GetWidth();

    //Texture coordinates
    mVData[ 0 ].texCoord.s = mTexLeft; mVData[ 0 ].texCoord.t = mTexTop;
    mVData[ 1 ].texCoord.s = mTexRight; mVData[ 1 ].texCoord.t = mTexTop;
    mVData[ 2 ].texCoord.s = mTexRight; mVData[ 2 ].texCoord.t = mTexBottom;
    mVData[ 3 ].texCoord.s = mTexLeft; mVData[ 3 ].texCoord.t = mTexBottom;

    //Vertex positions
    mHalfHeight=mRectCoord.GetHeight()/2;
    mHalfWidth=mRectCoord.GetWidth()/2;
    mHalfHeightN=mHalfHeight*-1;
    mHalfWidthN=mHalfWidth*-1;

    CalculateVerticies(mOldOrigin);

    mVBOID=0;
    mIBOID=0;
}


void LImage::CalculateVerticies(LOrigin origin){
    switch(origin){
        case L_ORIGIN_CENTER:
            mVData[ 0 ].position.x = mHalfWidthN; mVData[ 0 ].position.y = mHalfHeightN;
            mVData[ 1 ].position.x = mHalfWidth;  mVData[ 1 ].position.y = mHalfHeightN;
            mVData[ 2 ].position.x = mHalfWidth;  mVData[ 2 ].position.y = mHalfHeight;
            mVData[ 3 ].position.x = mHalfWidthN; mVData[ 3 ].position.y = mHalfHeight;
            break;

        case L_ORIGIN_TOP_LEFT:
            mVData[ 0 ].position.x = 0;                    mVData[ 0 ].position.y = 0;
            mVData[ 1 ].position.x = mRectCoord.GetWidth(); mVData[ 1 ].position.y = 0;
            mVData[ 2 ].position.x = mRectCoord.GetWidth(); mVData[ 2 ].position.y = mRectCoord.GetHeight();
            mVData[ 3 ].position.x = 0;                    mVData[ 3 ].position.y = mRectCoord.GetHeight();
            break;

        case L_ORIGIN_TOP_RIGHT:
            mVData[ 0 ].position.x = mRectCoord.GetWidth(); mVData[ 0 ].position.y = 0;
            mVData[ 1 ].position.x = 0;                    mVData[ 1 ].position.y = 0;
            mVData[ 2 ].position.x = 0;                    mVData[ 2 ].position.y = mRectCoord.GetHeight();
            mVData[ 3 ].position.x = mRectCoord.GetWidth(); mVData[ 3 ].position.y = mRectCoord.GetHeight();
            break;

        case L_ORIGIN_BOTTOM_RIGHT:
            mVData[ 0 ].position.x = mRectCoord.GetWidth(); mVData[ 0 ].position.y = mRectCoord.GetHeight();
            mVData[ 1 ].position.x = 0;                    mVData[ 1 ].position.y = mRectCoord.GetHeight();
            mVData[ 2 ].position.x = 0;                    mVData[ 2 ].position.y = 0;
            mVData[ 3 ].position.x = mRectCoord.GetWidth(); mVData[ 3 ].position.y = 0;
            break;

        case L_ORIGIN_BOTTOM_LEFT:
            mVData[ 0 ].position.x = 0;                    mVData[ 0 ].position.y = mRectCoord.GetHeight();
            mVData[ 1 ].position.x = mRectCoord.GetWidth(); mVData[ 1 ].position.y = mRectCoord.GetHeight();
            mVData[ 2 ].position.x = mRectCoord.GetWidth(); mVData[ 2 ].position.y = 0;
            mVData[ 3 ].position.x = 0;                    mVData[ 3 ].position.y = 0;
            break;
        }
}

void LImage::SetColorKey(unsigned int r, unsigned int g, unsigned int b){
    mTexture->SetColorKey(r,g,b);
}

LImage::LImage(){
    mTexture=NULL;
    mVBOID=0;
    mIBOID=0;
}

LImage::~LImage(){
    FreeVBO();
}


void LImage::InitVBO() {
    //If  VBO does not already exist
    if(mVBOID==0) {
        //LVertexData2D newData[ 4 ];
        GLuint iData[4];
        //Set rendering indices
        iData[0] = 0;
        iData[1] = 1;
        iData[2] = 2;
        iData[3] = 3;
        //Create VBO
        glGenBuffers( 1, &mVBOID );
        glBindBuffer( GL_ARRAY_BUFFER, mVBOID );
        glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(LVertexData2D), mVData, GL_DYNAMIC_DRAW );
        //Create IBO
        glGenBuffers( 1, &mIBOID );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIBOID );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), iData, GL_DYNAMIC_DRAW );
        //Unbind buffers
        glBindBuffer( GL_ARRAY_BUFFER, NULL ); glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, NULL );
    }
}

void LImage::FreeVBO() {
    //Free VBO and IBO
    if( mVBOID != 0 ){
        glDeleteBuffers( 1, &mVBOID );
        glDeleteBuffers( 1, &mIBOID );
        mVBOID=0;
        mIBOID=0;
    }
}

void LImage::Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color, LOrigin origin){
	if (mTexture){
        if(mOldOrigin!=origin){
            CalculateVerticies(origin);
            mOldOrigin=origin;
        }
        int drawX, drawY;

        switch(origin){
            case L_ORIGIN_CENTER:
                drawX=(x+mHalfWidth);
                drawY=(y+mHalfHeight);
                break;

            case L_ORIGIN_TOP_LEFT:
                drawX=(x);
                drawY=(y);
                break;

            case L_ORIGIN_TOP_RIGHT:
                drawX=(x+mHalfWidth-mHalfWidthN);
                drawY=(y);
                break;

            case L_ORIGIN_BOTTOM_RIGHT:
                drawX=(x+mHalfWidth-mHalfWidthN);
                drawY=(y+mHalfHeight-mHalfHeightN);
                break;

            case L_ORIGIN_BOTTOM_LEFT:
                drawX=(x);
                drawY=(y+mHalfHeight-mHalfHeightN);
                break;
        }

        glPushMatrix();

        glTranslatef(drawX,drawY,0);
        glRotatef(rotation, 0.0f, 0.0f, 1.0f);
        glScalef(scalingx, scalingy,1);

		mTexture->Bind();
		InitVBO();

        //Texture coordinates and Vertex Positions are already caclulated

        //Enable vertex and texture coordinate arrays
        //glEnableClientState( GL_VERTEX_ARRAY );
        //glEnableClientState( GL_TEXTURE_COORD_ARRAY );
         //Bind vertex buffer
         glBindBuffer( GL_ARRAY_BUFFER, GetVBOID() );

         //Update vertex buffer data (Commented out)
         //glBufferSubData( GL_ARRAY_BUFFER, 0, 4 * sizeof(LVertexData2D), mVData );

         //Set texture coordinate data
         glTexCoordPointer( 2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof( LVertexData2D, texCoord ) );
         //Set vertex data
         glVertexPointer( 2, GL_FLOAT, sizeof(LVertexData2D), (GLvoid*)offsetof( LVertexData2D, position ) );

         //color=L_COLOR_BLACK;
         glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, GetIBOID() );
         glColor4f(color.mR, color.mG, color.mB, color.mA);
         glDrawElements( GL_QUADS, 4, GL_UNSIGNED_INT, NULL );

        //glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        //glDisableClientState( GL_VERTEX_ARRAY );

		//Draw the textured rectangle.
		//glBegin(GL_QUADS);
		//glColor3f( 1.f, 1.f, 0.f );
		//glVertex3i(mHalfWidthN,mHalfHeightN,0);
		//glVertex3i(mHalfWidth,mHalfHeightN,0);
		//glVertex3i(mHalfWidth,mHalfHeight,0);
		//glVertex3i(mHalfWidthN,mHalfHeight,0);
		//glEnd();

		glPopMatrix();
	}
}
int globalInt=0;
void LImage::Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color,
                                                                LTexture* destination, LOrigin origin){
	if (mTexture){
        if(mOldOrigin!=origin){
            CalculateVerticies(origin);
            mOldOrigin=origin;
        }

        glPushMatrix();
            glLoadIdentity();
            glTranslatef(x,y,0);
            glRotatef(rotation, 0.0f, 0.0f, 1.0f);
            glScalef(scalingx, scalingy, 1);

            glColor4f(color.mR, color.mG, color.mB, color.mA);
            mTexture->RenderToTexture(mRectCoord, destination);
        glPopMatrix();

        //globalInt+=1;
		//if((globalInt<10)){
        //    std::string extension(".bmp");
        //    std::string num= NumberToString(globalInt);
        //    std::string exportPath="export//MEGAPOOGLE";
        //    exportPath.append(num);
        //    exportPath.append(extension);
        //   destination->ExportTexture(exportPath.c_str());
        //    //mTexture->ExportTexture("export//Megapp");
		//}
	}
}

LImage& LImage::operator =(const LImage& img){
    mTexture=img.mTexture;
    mRectCoord=img.mRectCoord;
    mOldOrigin=img.mOldOrigin;

    //Reset mVBOID and mIBOID
    FreeVBO();
    mVBOID=0;
    mIBOID=0;

    Initialize();
}*/

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

LAnimation::LAnimation(const double& spd){
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
    animations[aniName].SetColorKey(image, r,g,b);
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
    xml_node<>* imageNode;
    CRect imageBoundingBox(0,0,0,0);
    std::string animationName, speed;
    LAnimation* animation;
    CRect rect;
    for(; animationNode!=0; animationNode=animationNode->next_sibling()){ //Get all animations
        attribute = animationNode->first_attribute("name");
        if(attribute!=NULL){animationName=attribute->value();}

        attribute = animationNode->first_attribute("speed");
        if(attribute!=NULL){speed=attribute->value();}

        animations[animationName]=LAnimation((atof(speed.c_str())));
        animation=&animations[animationName];

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
    return true;
}

/*LSprite& LSprite::operator =(LSprite& spr){
    if (this==&spr) {return *this;}
    aniMapIt i;

    DeleteAnimations();
    i=spr.animations.begin();
    LAnimation* newani=NULL;
    for(; i!=spr.animations.end(); i++){
        newani = new LAnimation();
        *newani=*(i->second);
        animations[i->first]=newani;
    }

    //WRONG!!!!!!!!!!!!!!!!!!!!!!!!
    //currentAni=spr.currentAni;
    currentAni=animations.begin();

    frameProgression=spr.frameProgression;
    speed=spr.speed;
    spriteName=spr.spriteName;
    origin=spr.origin;
    return *this;
}*/
