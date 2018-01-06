#ifndef L_MAP
#define L_MAP

#include <rapidxml.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "RSC_Heightmap.h"
#include "RSC_Sprite.h"

#include "tiledMap/GID.h"
#include "tiledMap/Helpers.h"
#include "tiledMap/tiledSet.h"

#include "tiledMap/LayerImage.h"
#include "tiledMap/LayerObject.h"
#include "tiledMap/LayerTile.h"

// Forward declares
class RSC_MapImpl;
class TiledData;

// coupling here
class StateManager;
class GameState;
class ComponentCollisionManager;

/**
 * Encapsulates data from Tiled, TiledData owns sets, layers, and objects
 */
class TiledData {
  typedef std::map<int, MapEntrance> tEntrances;
  typedef std::map<int, MapExit>
      tExits;  // leave string blank ("") to refer to previous map

  // For the purposes of testing collision; introduces coupling, may want to
  // find a better solution
  friend ComponentCollisionManager;
  friend StateManager;
  friend GameState;
  friend RSC_MapImpl;

 public:
  TiledData(const unsigned int &tWidth, const unsigned int &tHeight);
  TiledData(const TiledData &rhs);
  ~TiledData();

  // returns false if the depth is already taken
  bool AddLayer(std::unique_ptr<TiledLayerGeneric> layer);
  void DeleteLayer(TiledLayerGeneric *layer);
  bool AddTileSet(std::unique_ptr<TiledSet> tileSet);

  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, const std::string &value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, double value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, int value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, bool value);

  TiledLayerGeneric *GetLayer(const std::string &name);

  unsigned int GetWidth() { return width; }
  unsigned int GetHeight() { return height; }

  GIDManager gid;
  const unsigned int width;
  const unsigned int height;
  const unsigned int tileWidth;
  const unsigned int tileHeight;
  int bgColor;

  // this data structure enables layers to be looked up by their name, this
  // structure does not assume ownership of the layers
  std::map<std::string, TiledLayerGeneric *> tiledLayers;
  // this data structure enables sets to be looked up by their name, this
  // structure does not assume ownership of the sets
  std::map<std::string, TiledSet *> tiledSets;
  // Can lookup via map depth
  // does not assume ownership of the layers
  std::map<MAP_DEPTH, TiledLayerGeneric *> tiledRenderableLayers;

  static std::unique_ptr<TiledData> LoadResourceFromTMX(
      const std::string &TMXname, const char *dat, unsigned int fsize);

  std::string GetProperty(const std::string &property);
  // Can access tiled layers by their name
  TiledTileLayer *GetTileLayer(const std::string &name);
  TiledImageLayer *GetImageLayer(const std::string &name);
  TiledObjectLayer *GetObjectLayer(const std::string &name);

  std::vector<TiledTileLayer *> GetSolidTileLayers();

 protected:
  // These variable names are brought to you by the redundancy department of
  // redundancy
  // All sorted by depth
  std::vector<std::unique_ptr<TiledSet>> tiledTileSets;
  std::vector<std::unique_ptr<TiledObjectLayer>> tiledObjectLayers;
  std::vector<std::unique_ptr<TiledImageLayer>> tiledImageLayers;
  std::vector<std::unique_ptr<TiledTileLayer>> tiledTileLayers;

  tExits mMapExits;
  tEntrances mMapEntrances;

 private:
  static std::unique_ptr<TiledSet> TMXLoadTiledSet(
      rapidxml::xml_node<> *tiledSetRootNode, const GID &firstGID,
      GIDManager &gidManager);
  static std::unique_ptr<TiledTileLayer> TMXLoadTiledTileLayer(
      rapidxml::xml_node<> *rootNode, const GIDManager &gidManager);
  static std::unique_ptr<TiledObjectLayer> TMXLoadTiledObjectLayer(
      rapidxml::xml_node<> *rootNode, TiledData *tiledData);
  static std::unique_ptr<TiledImageLayer> TMXLoadTiledImageLayer(
      rapidxml::xml_node<> *rootNode, const GIDManager &gidManager,
      int mapTilesW, int mapTilesH);

  XML_PropertyMap properties;
};

class ComponentScript;

/**
 * Interface for the Resource "Map"
 * Encapsulates a map used by the engine
 */
class RSC_Map {
 public:
  class Exception : public LEngineException {
    using LEngineException::LEngineException;
  };

  RSC_Map();
  virtual ~RSC_Map();
  virtual unsigned int GetWidthTiles() const = 0;
  virtual unsigned int GetHeightTiles() const = 0;
  virtual unsigned int GetWidthPixels() const = 0;
  virtual unsigned int GetHeightPixels() const = 0;

  virtual std::vector<TiledTileLayer *> GetSolidTileLayers() = 0;

  virtual std::string GetProperty(const std::string &property) const = 0;
  virtual std::string GetMapName() const = 0;
  virtual TiledTileLayer *GetTileLayer(const std::string &name) = 0;

  virtual std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, const std::string &value) = 0;
  virtual std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, double value) = 0;
  virtual std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, int value) = 0;
  virtual std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, bool value) = 0;
  // returns 0 if name doesn't exist
  virtual EID GetEIDFromName(const std::string &name) const = 0;

  /// \TODO remove 'GetTiledData' from RSC_MAP Interface, the user of this class
  /// should not need to access the Tiled Data
  virtual TiledData *GetTiledData() = 0;
  virtual void DeleteLayer(TiledLayerGeneric *layer) = 0;
};

class RSC_MapImpl : public RSC_Map {
  friend GameState;

 public:
  RSC_MapImpl(const std::string &mapName, std::unique_ptr<TiledData> td);
  RSC_MapImpl(const RSC_MapImpl &rhs);
  ~RSC_MapImpl();

  // Data
  std::unique_ptr<TiledData> tiledData;

  static std::unique_ptr<RSC_Map> LoadResource(const std::string &fname);
  void DeleteLayer(TiledLayerGeneric *layer);

  unsigned int GetWidthTiles() const;
  unsigned int GetHeightTiles() const;
  unsigned int GetWidthPixels() const;
  unsigned int GetHeightPixels() const;

  std::vector<TiledTileLayer *> GetSolidTileLayers();

  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, const std::string &value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, double value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, int value);
  std::vector<TiledLayerGeneric *> GetLayersWithProperty(
      const std::string &name, bool value);

  std::string GetProperty(const std::string &property) const;
  std::string GetMapName() const;
  TiledTileLayer *GetTileLayer(const std::string &name);
  EID GetEIDFromName(const std::string &name) const;

  TiledData *GetTiledData();

 private:
  const std::string mMapName;

  //! Every child's GID is incremented by the first (lowest) gid allowed for the
  //! map
  GID firstGID;
};

#endif
