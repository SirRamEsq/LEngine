#ifndef LENGINE_TILED_TILESET
#define LENGINE_TILED_TILESET

#include "GID.h"
#include "Helpers.h"
#include "../RSC_Texture.h"
#include "../RSC_Heightmap.h"
#include "../RSC_Sprite.h"

#include <map>
#include <string>

class RSC_MapImpl;
class TiledData;

//A tiled set defines what tiles are. Each tile set has a texture and also contains special properties of any tiles should have them.
//Each tile has a unique GID. Each tileSet has a Range of GIDs for each tile it defines
//It's up to the GID Manager to keep track of what ranges belong to which tilesets.
class TiledSet : public GIDEnabled{
    friend RSC_MapImpl;
    friend TiledData;

    public:
        TiledSet(const std::string& n, const std::string& tex, const unsigned int tileW, const unsigned int tileH, GID first, GIDManager* man);
        TiledSet(const TiledSet& rhs, GIDManager* g);
        void Init(GID first, GIDManager* man);

        void        LoadTexture                   ();
        void        LoadHeightMaps                (GID id);
        bool        ContainsTile                  (GID id) const;
        void        GetTextureCoordinatesFromGID  (GID id, float& left, float& right, float& top, float& bottom) const;
        const LAnimation* GetAnimationDataFromGID       (GID id) const;
        Rect       GetTextureRectFromGID         (GID id) const;

        RSC_Heightmap        GetHeightMap  (GID id) const {return tileHMAPs.find(id)->second; }
        int               GetTilesWide  ()       const {return tilesWide;     }
        int               GetTilesHigh  ()       const {return tilesHigh;     }
        int               GetTilesTotal ()       const {return tilesTotal;    }
        std::string       GetTextureName()       const {return textureName;   }
		std::string			GetTileProperty(GID id, const std::string& property) const;
        const RSC_Texture*   GetTexture    ()       const {return texture;       }

        std::string transparentColor;

        const unsigned int tileWidth;
        const unsigned int tileHeight;

        const std::string name;
        const std::string textureName;

    protected:
        std::map<GID, const LAnimation*> tileAnimations; //This can be directly accessed by RSC_Map (for the purpose of loading the animations in from a file)

    private:
        bool initializationOK;

        int tilesWide;
        int tilesHigh;
        int tilesTotal;

        std::map<GID, PropertyMap > tileProperties;

        const RSC_Texture* texture;

        //Only used if HMAP flags are set
        std::map<GID, RSC_Heightmap> tileHMAPs;
};

#endif
