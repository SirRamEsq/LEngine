#ifndef L_SPRITE
#define L_SPRITE

#include "RSC_Texture.h"
#include "../GenericContainer.h"

#include "rapidxml.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>

//SpriteContainer owns sprites
//sprites own animations
//animations own images
//images do not own textures

/*
class LImage{
    public:
        LImage(const std::string& fileName, const CRect& coord, LOrigin origin=L_ORIGIN_CENTER);
        LImage(RSC_Texture* texture, const CRect& coord, LOrigin origin=L_ORIGIN_CENTER);
        LImage();
        ~LImage();

        inline int GetWidth() {return mRectCoord.GetWidth();}
        inline int GetHeight(){return mRectCoord.GetHeight();}

        void Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color,
                                                                                    LOrigin origin=L_ORIGIN_CENTER);
        void Blit(int x, int y, double scalingx, double scalingy, double rotation, L_COLOR color, RSC_Texture* destination,
                                                                                    LOrigin origin=L_ORIGIN_CENTER);

        GLuint GetVBOID(){return mVBOID;}
        GLuint GetIBOID(){return mIBOID;}

        void CalculateVerticies(LOrigin origin);

        void SetColorKey(unsigned int r, unsigned int g, unsigned int b);

        uint8_t GetPixelAlpha(int x, int y){return mTexture->GetPixelAlpha(x,y);}

        LImage& operator =(const LImage& img);

        RSC_Texture* GetTexture(){return mTexture;}
        CRect GetCoords(){return mRectCoord;}
        LOrigin GetOrigin(){return mOldOrigin;}

    private:
        //Pre condition is that mTexture, mRectCoord, and mOldOrigin be set
        void Initialize();

        RSC_Texture*   mTexture;
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

        const CRect& GetCRectAtIndex(int index) const ;

        int GetWidth    (int index) const ;
        int GetHeight   (int index) const ;
        double GetSpeed ()          const {return defaultSpeed;}

        int NumberOfImages() const {return images.size();}

        void SetColorKey(int image, unsigned int r, unsigned int g, unsigned int b);

        const AnimationLoadTag loadTag;

    protected:
        void DeleteImages();
		bool ValidateIndex(int index) const;
        imageVec images;
        int currentImage;

        double defaultSpeed;
};

class RSC_Sprite{
    typedef std::map<std::string, LAnimation> aniMap;
	typedef aniMap::iterator aniMapIt;

    public:
        RSC_Sprite(const std::string& sname);
        ~RSC_Sprite();

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

        static std::unique_ptr<RSC_Sprite> LoadResource(const std::string& fname);

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
