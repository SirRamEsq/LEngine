#ifndef L_TDF
#define L_TDF

#include "../Defines.h"
#include "LSprite.h"
#include "LHeightmap.h"
#include <algorithm>
#include <math.h>
#include <string>

class TileMap; //forward declare

class LTDF{
    /*friend class TileMap;
    public:
        LTDF(std::string tdfn, std::string sprn, std::string anin, std::string hmName, double frict, int32_t flgs);
        ~LTDF();

        uint32_t GetFlags(){return flags;}
        //funcitons are used lua side
        bool IsSolid(){return ((flags & TF_solid)==TF_solid);}
        //bool ColUp(){return ((flags & TF_collisionUp)==TF_collisionUp);}
        //bool ColDown(){return ((flags & TF_collisionDown)==TF_collisionDown);}
        //bool ColLeft(){return ((flags & TF_collisionLeft)==TF_collisionLeft);}
        //bool ColRight(){return ((flags & TF_collisionRight)==TF_collisionRight);}
        bool UseHeightMap(){return ((flags & TF_useHMap)==TF_useHMap);}

        int GetAngleH();
        int GetAngleV();

        int GetHeightAtX(int x);
        int GetHeightAtY(int y);

        void ReloadHeightmap();

        void LoadSprite();
        LSprite* GetSprite(){return sprite;}

        inline std::string GetTDFName(){return tdfName;}
        inline std::string GetAnimationName(){return animationName;}
        inline std::string GetSpriteName(){return spriteName;}

        inline double GetFriction(){return friction;}

        LTDF& operator =(LTDF& tdf);

    protected:
        std::string tdfName;
        std::string animationName;
        std::string spriteName;
        std::string heightmapName;

        LSprite* sprite;

    private:
        double friction;
        int32_t flags;
        LHeightmap* hmap;*/
};

#endif
