#ifndef L_ENGINE_RSC_MAP_MOCK
#define L_ENGINE_RSC_MAP_MOCK

#include "../../Engine/Resources/RSC_Map.h"

class RSC_MapMock : public RSC_Map{
    public:
        RSC_MapMock(const std::string& mName, int w, int h);
        ~RSC_MapMock();

        int widthTiles;
        int heightTiles;

        const int tileWidth  = 16;
        const int tileHeight = 16;

        int GetWidthTiles() const;
        int GetHeightTiles() const;
        int GetWidthPixels() const;
        int GetHeightPixels() const;

        std::string GetProperty (const std::string& property) const;
        std::string GetMapName  () const;
        TiledTileLayer* GetTileLayer(const std::string& name);
        EID GetEIDFromName(const std::string& name) const;

        const TiledTileLayer* GetTileLayerCollision(int x, int y, bool areTheseTileCoords) const;

        TiledData* GetTiledData();

        const std::string mapName;

        GIDManager gid;
        TiledTileLayer layer;
};

#endif // L_ENGINE_RSC_MAP_MOCK
