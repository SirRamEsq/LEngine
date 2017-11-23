#include "LayerGeneric.h"

TiledLayerGeneric::TiledLayerGeneric(const unsigned int &tileW,
                                     const unsigned int &tileH,
                                     const std::string &name,
                                     const MAP_DEPTH &depth,
                                     const GIDManager *g,
                                     const L_TILED_LAYER_TYPE &type)
    : tileWidth(tileW),
      tileHeight(tileH),
      pixelWidth(tileW * 16),
      pixelHeight(tileH * 16),
      layerName(name),
      layerDepth(depth),
	  mShaderFrag(""),
	  mShaderVert(""),
	  mShaderGeo(""),
      GIDM(g),
      layerType(type) {}

bool TiledLayerGeneric::Ignore() const {
  std::string propertyName = "IGNORE";
  if (PropertyExists(propertyName) == false) {
    return false;
  }
  return (GetPropertyValue(propertyName) != "false");
}

std::string TiledLayerGeneric::GetPropertyValue(
    const std::string &propertyName) const {
  auto i = properties.find(propertyName);
  if (i == properties.end()) {
    return "";
  }

  return std::get<1>(i->second);
}

bool TiledLayerGeneric::PropertyExists(const std::string &propertyName) const {
  return (properties.find(propertyName) != properties.end());
}
