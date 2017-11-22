#ifndef LENGINE_TILED_LAYER_GENERIC
#define LENGINE_TILED_LAYER_GENERIC

#include "GID.h"
#include "Helpers.h"

class RSC_MapImpl;
class TiledData;

// A TiledTileLayer defines the make up of a layer of terrain from the TiledSet
// that it makes use of
// The TiledTileLayer is, at it's core, a 2d array of GID values from a single
// TileSet.
// In addition to the properties outlined in the TiledSet, the TiledTileLayer
// can override and add to those properties.
class TiledLayerGeneric {
  friend RSC_MapImpl;
  friend TiledData;

 public:
  TiledLayerGeneric(const unsigned int &tileW, const unsigned int &tileH,
                    const std::string &name, const MAP_DEPTH &depth,
                    const GIDManager *g,
                    const L_TILED_LAYER_TYPE &type = LAYER_GENERIC);
  virtual ~TiledLayerGeneric() {}

  // Width and Height in Pixels
  const unsigned int pixelWidth;
  const unsigned int pixelHeight;

  // Width and Height in Tiles
  const unsigned int tileWidth;
  const unsigned int tileHeight;

  const L_TILED_LAYER_TYPE layerType;
  const std::string layerName;

  // GID Manager for the RSC_Map that this layer is a part of
  const GIDManager *GIDM;

  float GetAlpha() const { return layerOpacity; }
  int GetFlags() const { return layerFlags; }
  void SetAlpha(float a) { layerOpacity = a; }
  bool IsVisible() const { return layerVisible; }
  bool Ignore() const;
  MAP_DEPTH GetDepth() const { return layerDepth; }

  std::string GetPropertyValue(const std::string &propertyName) const;
  bool PropertyExists(const std::string &propertyName) const;

 protected:
  int layerFlags;
  MAP_DEPTH layerDepth;
  float layerOpacity;
  bool layerVisible;

  XML_PropertyMap properties;
};

#endif
