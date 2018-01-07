#include "RSC_MapMock.h"

RSC_MapMock::RSC_MapMock(const std::string &mName, unsigned int w,
                         unsigned int h)
    : gid(),
      mapName(mName),
      widthTiles(w),
      heightTiles(h),
      layer(w, h, "LAYER", 0, &gid) {}

RSC_MapMock::~RSC_MapMock() {}

EID RSC_MapMock::GetEIDFromName(const std::string &name) const { return 0; }

std::string RSC_MapMock::GetMapName() const { return mapName; }
TiledData *RSC_MapMock::GetTiledData() { return NULL; }

unsigned int RSC_MapMock::GetWidthTiles() const { return widthTiles; }
unsigned int RSC_MapMock::GetHeightTiles() const { return heightTiles; }
unsigned int RSC_MapMock::GetWidthPixels() const {
  return widthTiles * tileWidth;
}
unsigned int RSC_MapMock::GetHeightPixels() const {
  return heightTiles * tileHeight;
}

std::string RSC_MapMock::GetProperty(const std::string &property) const {
  return std::string("VALUE");
}

TiledTileLayer *RSC_MapMock::GetTileLayer(const std::string &name) {
  return NULL;
}

std::vector<TiledTileLayer *> RSC_MapMock::GetSolidTileLayers() {
  std::vector<TiledTileLayer *> layers;
  layers.push_back(&layer);
  return layers;
}

const TiledTileLayer *RSC_MapMock::GetTileLayerCollision(
    unsigned int x, unsigned int y, bool areTheseTileCoords) const {
  // Return first tile layer collided with
  if (!areTheseTileCoords) {
    CoordToGrid(x, y);
  }

  // no bounds checking, just passing it straight into the layer GetGID function
  if (layer.GetGID(x, y) != 0) {
    return &layer;
  }

  return NULL;
}
std::vector<TiledLayerGeneric *> RSC_MapMock::GetLayersWithProperty(
    const std::string &name, const std::string &value) {
  std::vector<TiledLayerGeneric *> layers;
  return layers;
}
std::vector<TiledLayerGeneric *> RSC_MapMock::GetLayersWithProperty(
    const std::string &name, double value) {
  std::vector<TiledLayerGeneric *> layers;
  return layers;
}
std::vector<TiledLayerGeneric *> RSC_MapMock::GetLayersWithProperty(
    const std::string &name, int value) {
  std::vector<TiledLayerGeneric *> layers;
  return layers;
}
std::vector<TiledLayerGeneric *> RSC_MapMock::GetLayersWithProperty(
    const std::string &name, bool value) {
  std::vector<TiledLayerGeneric *> layers;
  return layers;
}
