#include "LayerImage.h"

TiledImageLayer::TiledImageLayer(const unsigned int &tileW,
                                 const unsigned int &tileH,
                                 const std::string &name,
                                 const MAP_DEPTH &depth, const GIDManager *g,
                                 const RSC_Texture *tex)
    : TiledLayerGeneric(tileW, tileH, name, depth, g, LAYER_IMAGE),
      texture(tex) {
  offset = Coord2df(0, 0);
  parallax = Coord2df(1, 1);
  repeatX = false;
  repeatY = false;
  stretchToMapX = false;
  stretchToMapY = false;
}

void TiledImageLayer::SetOffset(const Coord2df &off) { offset = off.Round(); }

Coord2df TiledImageLayer::GetOffset() const { return offset; }

void TiledImageLayer::SetTexture(const RSC_Texture *tex) { texture = tex; }

const RSC_Texture *TiledImageLayer::GetTexture() const { return texture; }

Coord2df TiledImageLayer::GetParallax() const { return parallax; }

void TiledImageLayer::SetParallax(const Coord2df &para) { parallax = para; }

TiledImageLayer::TiledImageLayer(const TiledImageLayer &rhs,
                                 const GIDManager *g)
    : TiledLayerGeneric(rhs.tileWidth, rhs.tileHeight, rhs.layerName,
                        rhs.GetDepth(), g, rhs.layerType),
      texture(rhs.texture) {
  layerFlags = rhs.layerFlags;
  layerDepth = rhs.layerDepth;
  layerOpacity = rhs.layerOpacity;
  layerVisible = rhs.layerVisible;
  CopyPropertyMap(rhs.properties, properties);
  offset = rhs.offset;
  parallax = rhs.parallax;
  repeatX = rhs.repeatX;
  repeatY = rhs.repeatY;
  stretchToMapX = rhs.stretchToMapX;
  stretchToMapY = rhs.stretchToMapY;
}

bool TiledImageLayer::GetRepeatX() const { return repeatX; }
void TiledImageLayer::SetRepeatX(bool val) { repeatX = val; }
bool TiledImageLayer::GetRepeatY() const { return repeatY; }
void TiledImageLayer::SetRepeatY(bool val) { repeatY = val; }

bool TiledImageLayer::GetStretchToMapX() const { return stretchToMapX; }
void TiledImageLayer::SetStretchToMapX(bool val) { stretchToMapX = val; }
bool TiledImageLayer::GetStretchToMapY() const { return stretchToMapY; }
void TiledImageLayer::SetStretchToMapY(bool val) { stretchToMapY = val; }
