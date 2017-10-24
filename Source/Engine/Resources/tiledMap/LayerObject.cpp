#include "LayerObject.h"

TiledObjectLayer::TiledObjectLayer(const unsigned int &w, const unsigned int &h,
                                   const std::string &name,
                                   const MAP_DEPTH &depth, const GIDManager *g)
    : TiledLayerGeneric(w, h, name, depth, g, LAYER_OBJECT) {}

TiledObjectLayer::TiledObjectLayer(const TiledObjectLayer &rhs,
                                   const GIDManager *g)
    : TiledLayerGeneric(rhs.tileWidth, rhs.tileHeight, rhs.layerName,
                        rhs.GetDepth(), g, rhs.layerType) {
  for (auto i = rhs.objects.begin(); i != rhs.objects.end(); i++) {
    objects[i->first] = i->second;
  }
  layerFlags = rhs.layerFlags;
  layerDepth = rhs.layerDepth;
  layerOpacity = rhs.layerOpacity;
  layerVisible = rhs.layerVisible;
  CopyPropertyMap(rhs.properties, properties);
}
