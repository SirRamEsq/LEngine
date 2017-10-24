#ifndef LENGINE_TILED_LAYER_TILE
#define LENGINE_TILED_LAYER_TILE

#include "LayerGeneric.h"
#include "tiledSet.h"

class TiledTileLayer : public TiledLayerGeneric {
  friend RSC_MapImpl;
  friend TiledData;

 public:
  TiledTileLayer(const unsigned int &w, const unsigned int &h,
                 const std::string &name, const MAP_DEPTH &depth,
                 const GIDManager *g);
  TiledTileLayer(const TiledTileLayer &rhs, const GIDManager *g);

  /**
   * Gets GID at Tile-Coordinate x,y
   */
  GID GetGID(unsigned int x, unsigned int y) const;
  /**
   * Sets GID at Tile-Coordinate x,y
   */
  void SetGID(unsigned int x, unsigned int y, GID id) const;
  /**
   * Checks if GID at Tile-Coordinate x,y is not 0
   */
  bool HasTile(unsigned int x, unsigned int y) const;

  /// Calls 'GetTileProperty' from tiledset
  std::string GetTileProperty(GID id, const std::string &property) const;

  // LuaInterface Functions
  unsigned int GetTile(unsigned int x, unsigned int y) const;
  void SetTile(unsigned int x, unsigned int y, unsigned int id) const;

  // Lua Functions
  bool IsSolid() const;
  bool IsDestructible() const;
  bool UsesHMaps() const;
  float GetFriction() const;
  // All set tiles will be updated on the GPU
  void UpdateRenderArea(Rect area) const;

  unsigned int GetAnimationRate() const { return animatedRefreshRate; }

  TiledSet *GetTiledSet() const { return tileSet; }

  mutable std::vector<Rect> updatedAreas;

 protected:
  // Each tiled Layer is allowed one tiled set
  TiledSet *tileSet;

  float friction;
  unsigned int animatedRefreshRate;

 private:
  mutable std::vector<std::vector<GID>> data2D;

  void InitializeMap();
  bool inline IsValidXY(unsigned int x, unsigned int y) const;
};

#endif
