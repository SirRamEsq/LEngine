#ifndef L_SPRITE
#define L_SPRITE

#include "../Defines.h"
#include "LTexture.h"

#include "rapidxml.hpp"

#include <string>
#include <vector>
#include <map>

//SpriteContainer owns sprites
//sprites own animations
//animations own images
//images do not own textures

class Kernel;//Forward Declare
/*
class LImage{
    public:
        LImage(const std::string& fileName, const CRect& coord, LOrigin origin=L_ORIGIN_CENTER);
        LImage(LTexture* texture, const CRect& coord, LOrigin origin=L_ORIGIN_CENTER);
        LImage();
        ~LImage();

        inline int GetWidth() {return mRectCoord.GetWidth();}
        inline int GetHeight(){return mRectCoord.GetHeight();}

        void Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color,
                                                                                    LOrigin origin=L_ORIGIN_CENTER);
        void Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color, LTexture* destination,
                                                                                    LOrigin origin=L_ORIGIN_CENTER);

        GLuint GetVBOID(){return mVBOID;}
        GLuint GetIBOID(){return mIBOID;}

        void CalculateVerticies(LOrigin origin);

        void SetColorKey(unsigned int r, unsigned int g, unsigned int b);

        uint8_t GetPixelAlpha(int x, int y){return mTexture->GetPixelAlpha(x,y);}

        LImage& operator =(const LImage& img);

        LTexture* GetTexture(){return mTexture;}
        CRect GetCoords(){return mRectCoord;}
        LOrigin GetOrigin(){return mOldOrigin;}

    private:
        //Pre condition is that mTexture, mRectCoord, and mOldOrigin be set
        void Initialize();

        LTexture*   mTexture;
        CRect  mRectCoord;
        LVertexData2D mVData[ 4 ];

        LOrigin mOldOrigin;

        //-Binds NULL VBO and IBO
        void InitVBO();
        void FreeVBO();

        //Vertex and Index Buffer IDs
        GLuint mVBOID;
        GLuint mIBOID;

        float mTexTop;
		float mTexBottom;
		float mTexLeft;
		float mTexRight;

		int mHalfHeight, mHalfWidth, mHalfHeightN, mHalfWidthN;
};*/

enum AnimationLoadTag{
    LOAD_TAG_UNK = 0,
    LOAD_TAG_ANIMATION           = 1,
    LOAD_TAG_ANIMATION_SEQUENCE  = 2
};

class LAnimation{
    typedef std::vector<CRect> imageVec;

    public:
        LAnimation(const double&, AnimationLoadTag t);

        ~LAnimation();
        void Clear();

        void AppendImage(const CRect& img);

        imageVec::iterator ItBegin() {return images.begin();}
        imageVec::iterator ItEnd() {return images.end();}

        const CRect& GetCRectAtIndex(const int& index) const ;

        int GetWidth    (int index) const ;
        int GetHeight   (int index) const ;
        double GetSpeed ()          const {return defaultSpeed;}

        int NumberOfImages() const {return images.size();}

        void SetColorKey(int image, unsigned int r, unsigned int g, unsigned int b);

        const AnimationLoadTag loadTag;

    protected:
        void DeleteImages();
        imageVec images;
        int currentImage;

        double defaultSpeed;
};

class LSprite{
    typedef std::map<std::string, LAnimation> aniMap;
	typedef aniMap::iterator aniMapIt;

    public:
        LSprite(const std::string& sname);
        ~LSprite();

        std::string GetName(){return spriteName;}

        const LAnimation* GetAnimation(const std::string& aniName) const;

        aniMapIt ItBegin() {return animations.begin();}
        aniMapIt ItEnd()   {return animations.end();}

        LOrigin GetOrigin() const {return origin;}
        void SetOrigin(LOrigin o);

        std::string GetTextureName() const {return mTextureName;}

        void SetColorKey(const std::string& aniName, unsigned int image, unsigned int r, unsigned int g, unsigned int b);

        const aniMap* GetAnimationMapPointer() const {return &animations;}

        bool LoadFromXML(const char* dat, unsigned int fsize);

        int GetWidth () const {return width; }
        int GetHeight() const {return height;}

        int GetTransparentColorRed   () const {return transparentColorRed;  }
        int GetTransparentColorGreen () const {return transparentColorGreen;}
        int GetTransparentColorBlue  () const {return transparentColorBlue; }

    protected:
        void DeleteAnimations();

        LOrigin origin;

        aniMap animations;
        std::string spriteName;
        std::string mTextureName;


        int width, height; //for calculating center of image
        int transparentColorRed;
        int transparentColorGreen;
        int transparentColorBlue;

    private:
        void LoadAnimation(rapidxml::xml_node<>* animationNode);
        void LoadAnimationSequence(rapidxml::xml_node<>* animationNode);
};

#endif
