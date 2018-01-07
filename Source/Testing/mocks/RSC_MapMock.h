#ifndef L_ENGINE_RSC_MAP_MOCK
#define L_ENGINE_RSC_MAP_MOCK

#include "../../Engine/Resources/RSC_Map.h"

class RSC_MapMock : public RSC_Map {
 public:
  RSC_MapMock(const std::string &mName, unsigned int w, unsigned int h);
  ~RSC_MapMock();

  unsigned int widthTiles;
  unsigned int heightTiles;

  const unsigned int tileWidth = 16;
  const unsigned int tileHeight = 16;

  unsigned int GetWidthTiles() const;
  unsigned int GetHeightTiles() const;
  unsigned int GetWidthPixels() const;
  unsigned int GetHeightPixels() const;

  Vec3 GetAmbientLight(){return Vec3();}
  void SetAmbientLight(Vec3 light) {}
  void DeleteLayer(TiledLayerGeneric *layer){}

  std::vector<TiledTileLayer *> GetSolidTileLayers();

  std::string GetProperty(const std::string &property) const;
  std::string GetMapName() const;
  TiledTileLayer *GetTileLayer(const std::string &name);
  EID GetEIDFromName(const std::string &name) const;

  const TiledTileLayer *GetTileLayerCollision(unsigned int x, unsigned int y,
                                              bool areTheseTileCoords) const;

  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, const std::string &value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, double value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, int value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, bool value);

  TiledData *GetTiledData();

  const std::string mapName;

  GIDManager gid;
  TiledTileLayer layer;
};

#endif  // L_ENGINE_RSC_MAP_MOCK
