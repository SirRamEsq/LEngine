#include "RSC_Map.h"
#include "../Exceptions.h"
#include "../Kernel.h"

#include "../TiledProperties.h"

#include <sstream>
#include <stdlib.h>
/////////////
// TiledData//
/////////////

TiledData::TiledData(const unsigned int &tWidth, const unsigned int &tHeight)
    : tileWidth(tWidth),
      tileHeight(tHeight),
      width(tWidth * 16),
      height(tHeight * 16) {}

TiledData::TiledData(const TiledData &rhs)
    : tileWidth(rhs.tileWidth),
      tileHeight(rhs.tileHeight),
      width(rhs.width),
      height(rhs.height) {
  bgColor = rhs.bgColor;
  gid = rhs.gid;

  for (auto i = rhs.tiledTileSets.begin(); i != rhs.tiledTileSets.end(); i++) {
    std::unique_ptr<TiledSet> newSet(new TiledSet(*(i->get()), &gid));

    AddTileSet(std::unique_ptr<TiledSet>(newSet.release()));
  }
  for (auto i = rhs.tiledImageLayers.begin(); i != rhs.tiledImageLayers.end();
       i++) {
    std::unique_ptr<TiledLayerGeneric> layer(
        new TiledImageLayer(*(i->get()), &gid));

    AddLayer(std::unique_ptr<TiledLayerGeneric>(layer.release()));
  }
  for (auto i = rhs.tiledTileLayers.begin(); i != rhs.tiledTileLayers.end();
       i++) {
    std::unique_ptr<TiledLayerGeneric> layer(
        new TiledTileLayer(*(i->get()), &gid));

    AddLayer(std::move(layer));
  }
  for (auto i = rhs.tiledObjectLayers.begin(); i != rhs.tiledObjectLayers.end();
       i++) {
    std::unique_ptr<TiledLayerGeneric> layer(
        new TiledObjectLayer(*(i->get()), &gid));

    AddLayer(std::unique_ptr<TiledLayerGeneric>(layer.release()));
  }

  CopyPropertyMap(rhs.properties, properties);

  mAmbientLight = rhs.mAmbientLight;
}

TiledData::~TiledData() {}

bool TiledData::AddTileSet(std::unique_ptr<TiledSet> tileSet) {
  if (tileSet.get() == NULL) {
    return false;
  }
  tiledSets[tileSet->name] = tileSet.get();
  tiledTileSets.push_back(std::unique_ptr<TiledSet>(tileSet.release()));
  return true;
}

template <class T>
bool SortLayersByDepth(T &l1, T &l2) {
  // Lowest First, Highest Last
  return (l1->GetDepth() < l2->GetDepth());
}

bool TiledData::AddLayer(std::unique_ptr<TiledLayerGeneric> layer) {
  if (layer.get() == NULL) {
    return false;
  }
  MAP_DEPTH depth = layer->GetDepth();
  auto layerType = layer->layerType;

  tiledLayers[layer->layerName] = layer.get();
  // Make sure depth isn't already taken if the layer is supposed to be rendered
  if ((layerType == LAYER_IMAGE) or (layerType == LAYER_TILE)) {
    // Ignore layer if ignore flag is set
    if (layer->Ignore() == true) {
      return true;
    }

    if (tiledRenderableLayers.find(depth) != tiledRenderableLayers.end()) {
      // if depth already exists, return 0 and implicitly delete layer via smart
      // pointer
      std::stringstream ss;
      ss << "Couldn't add TileLayer named: " << layer->layerName
         << "\n    With Depth of: " << depth << "\n    Depth already taken";
      LOG_INFO(ss.str());
      return false;
    }

    // Add layer to map if depth isn't already taken
    tiledRenderableLayers[depth] = layer.get();

    if (layerType == LAYER_IMAGE) {
      tiledImageLayers.push_back(
          std::unique_ptr<TiledImageLayer>((TiledImageLayer *)layer.release()));
      // Sort Layers by Depth
      std::sort(tiledImageLayers.begin(), tiledImageLayers.end(),
                SortLayersByDepth<std::unique_ptr<TiledImageLayer> >);
    } else if (layerType == LAYER_TILE) {
      tiledTileLayers.push_back(
          std::unique_ptr<TiledTileLayer>((TiledTileLayer *)layer.release()));
      // Sort Layers by Depth
      std::sort(tiledTileLayers.begin(), tiledTileLayers.end(),
                SortLayersByDepth<std::unique_ptr<TiledTileLayer> >);
    }
  } else if (layerType == LAYER_OBJECT) {
    tiledObjectLayers.push_back(
        std::unique_ptr<TiledObjectLayer>((TiledObjectLayer *)layer.release()));
    // Sort Layers by Depth
    std::sort(tiledObjectLayers.begin(), tiledObjectLayers.end(),
              SortLayersByDepth<std::unique_ptr<TiledObjectLayer> >);
  }

  return true;
}

void TiledData::DeleteLayer(TiledLayerGeneric *layer) {
  for (auto i = tiledImageLayers.begin(); i != tiledImageLayers.end(); i++) {
    if (i->get() == layer) {
      // tiledImageLayers.erase(i);
      return;
    }
  }

  for (auto i = tiledTileLayers.begin(); i != tiledTileLayers.end(); i++) {
    if (i->get() == layer) {
      auto tileLayer = (TiledTileLayer *)(layer);
      tileLayer->ClearTiles();
      return;
    }
  }

  for (auto i = tiledObjectLayers.begin(); i != tiledObjectLayers.end(); i++) {
    if (i->get() == layer) {
      // tiledObjectLayers.erase(i);
      return;
    }
  }
}

RSC_Map::RSC_Map() {}
RSC_Map::~RSC_Map() {}

std::string TiledTileLayer::GetTileProperty(GID id,
                                            const std::string &property) const {
  if (id == 0) {
    return "";
  }
  return tileSet->GetTileProperty(id, property);
}
////////
// RSC_MapImpl//
////////

RSC_MapImpl::RSC_MapImpl(const std::string &mapName,
                         std::unique_ptr<TiledData> td)
    : mMapName(mapName) {
  tiledData.reset();
  tiledData = std::move(td);
}

EID RSC_MapImpl::GetEIDFromName(const std::string &name) const { return 0; }

std::string RSC_MapImpl::GetMapName() const { return mMapName; }
TiledData *RSC_MapImpl::GetTiledData() { return tiledData.get(); }

RSC_MapImpl::RSC_MapImpl(const RSC_MapImpl &rhs) : mMapName(rhs.mMapName) {
  firstGID = rhs.firstGID;

  tiledData = std::make_unique<TiledData>(*rhs.tiledData.get());
}

RSC_MapImpl::~RSC_MapImpl() {}

unsigned int RSC_MapImpl::GetWidthTiles() const { return tiledData->tileWidth; }
unsigned int RSC_MapImpl::GetHeightTiles() const {
  return tiledData->tileWidth;
}
unsigned int RSC_MapImpl::GetWidthPixels() const { return tiledData->width; }
unsigned int RSC_MapImpl::GetHeightPixels() const { return tiledData->height; }

std::string RSC_MapImpl::GetProperty(const std::string &property) const {
  return tiledData->GetProperty(property);
}

std::string TiledData::GetProperty(const std::string &property) {
  auto i = properties.find(property);
  if (i == properties.end()) {
    return "";
  }
  return i->second.second;
}

TiledTileLayer *RSC_MapImpl::GetTileLayer(const std::string &property) {
  return tiledData->GetTileLayer(property);
}

std::vector<TiledLayerGeneric *> RSC_MapImpl::GetLayersWithProperty(
    const std::string &name, const std::string &value) {
  return tiledData->GetLayersWithProperty(name, value);
}
std::vector<TiledLayerGeneric *> RSC_MapImpl::GetLayersWithProperty(
    const std::string &name, double value) {
  return tiledData->GetLayersWithProperty(name, value);
}
std::vector<TiledLayerGeneric *> RSC_MapImpl::GetLayersWithProperty(
    const std::string &name, int value) {
  return tiledData->GetLayersWithProperty(name, value);
}
std::vector<TiledLayerGeneric *> RSC_MapImpl::GetLayersWithProperty(
    const std::string &name, bool value) {
  return tiledData->GetLayersWithProperty(name, value);
}

std::vector<TiledTileLayer *> RSC_MapImpl::GetSolidTileLayers() {
  return tiledData->GetSolidTileLayers();
}

std::vector<TiledTileLayer *> TiledData::GetSolidTileLayers() {
  std::vector<TiledTileLayer *> layers;

  for (auto i = tiledTileLayers.begin(); i != tiledTileLayers.end(); i++) {
    if (i->get()->IsSolid()) {
      layers.push_back(i->get());
    }
  }

  return layers;
}

TiledTileLayer *TiledData::GetTileLayer(const std::string &name) {
  auto layer = GetLayer(name);
  if (layer != NULL) {
    if (layer->layerType == LAYER_TILE) {
      return (TiledTileLayer *)layer;
    }
  }
  return NULL;
}
TiledImageLayer *TiledData::GetImageLayer(const std::string &name) {
  auto layer = GetLayer(name);
  if (layer != NULL) {
    if (layer->layerType == LAYER_IMAGE) {
      return (TiledImageLayer *)layer;
    }
  }
  return NULL;
}
TiledObjectLayer *TiledData::GetObjectLayer(const std::string &name) {
  auto layer = GetLayer(name);
  if (layer != NULL) {
    if (layer->layerType == LAYER_OBJECT) {
      return (TiledObjectLayer *)layer;
    }
  }
  return NULL;
}

std::vector<TiledLayerGeneric *> TiledData::GetLayersWithProperty(
    const std::string &name, const std::string &value) {
  std::vector<TiledLayerGeneric *> layers;
  for (auto i = tiledLayers.begin(); i != tiledLayers.end(); i++) {
    if (i->second->PropertyExists(name)) {
      auto stringValue = i->second->GetPropertyValue(name);
      if (value == stringValue) {
        layers.push_back(i->second);
      }
    }
  }
  return layers;
}
std::vector<TiledLayerGeneric *> TiledData::GetLayersWithProperty(
    const std::string &name, double value) {
  std::vector<TiledLayerGeneric *> layers;
  for (auto i = tiledLayers.begin(); i != tiledLayers.end(); i++) {
    if (i->second->PropertyExists(name)) {
      auto stringValue = i->second->GetPropertyValue(name);
      float floatValue = StringToNumber<float>(stringValue);
      if (value == floatValue) {
        layers.push_back(i->second);
      }
    }
  }
  return layers;
}
std::vector<TiledLayerGeneric *> TiledData::GetLayersWithProperty(
    const std::string &name, int value) {
  std::vector<TiledLayerGeneric *> layers;
  for (auto i = tiledLayers.begin(); i != tiledLayers.end(); i++) {
    if (i->second->PropertyExists(name)) {
      auto stringValue = i->second->GetPropertyValue(name);
      int intValue = StringToNumber<int>(stringValue);
      if (value == intValue) {
        layers.push_back(i->second);
      }
    }
  }
  return layers;
}
std::vector<TiledLayerGeneric *> TiledData::GetLayersWithProperty(
    const std::string &name, bool value) {
  std::vector<TiledLayerGeneric *> layers;
  for (auto i = tiledLayers.begin(); i != tiledLayers.end(); i++) {
    if (i->second->PropertyExists(name)) {
      auto stringValue = i->second->GetPropertyValue(name);
      auto boolValue = (stringValue == "true");
      if (value == boolValue) {
        layers.push_back(i->second);
      }
    }
  }
  return layers;
}

TiledLayerGeneric *TiledData::GetLayer(const std::string &name) {
  return tiledLayers[name];
}

Vec3 TiledData::GetAmbientLight() { return mAmbientLight; }
void TiledData::SetAmbientLight(Vec3 light) { mAmbientLight = light; }

std::unique_ptr<RSC_Map> RSC_MapImpl::LoadResource(const std::string &fname) {
  std::unique_ptr<RSC_MapImpl> rscMap = NULL;
  try {
    std::string fullPath = "Resources/Maps/" + fname;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      throw LEngineFileException("Couldn't load RSC_MapImpl from path", fname);
    }

    try {
      auto tiledData = TiledData::LoadResourceFromTMX(
          fname, data.get()->GetData(), data.get()->length);
      rscMap = std::make_unique<RSC_MapImpl>(fname, std::move(tiledData));
    } catch (RSC_Map::Exception e) {
      LOG_INFO(e.what());
      throw e;
    }
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  return std::move(rscMap);
}

std::unique_ptr<TiledData> TiledData::LoadResourceFromTMX(
    const std::string &TMXname, const char *dat, unsigned int fsize) {
  // There are three primary data structures this function looks for
  // TileSets
  // TileLayers (Large 2D array that references TileSets)
  // ObjectLayers
  // Tilesets are always first in the xml file, then come the layers (object and
  // tile) in order of depth

  std::string XML = std::string(dat, fsize);

  std::stringstream mapInitializeDebugMessage;
  mapInitializeDebugMessage << "Loading TMX Map: " << TMXname;

  LOG_INFO(mapInitializeDebugMessage.str());
  using namespace rapidxml;
  xml_document<> doc;  // character type defaults to char
  XML_AttributeMap
      attributes;  // this will contain all of the attributes of a given
                   // part of the map as the function goes on
  // Convert const char to char

  // WILL CAUSE PROBLEMS IF std::string XML IS USED BEYOND THIS POINT
  doc.parse<0>((char *)(XML.c_str()));  // 0 means default parse flags

  std::string errorString;

  // Find Specific Node
  xml_node<> *node = doc.first_node("map");

  std::string testString;
  std::string valueString;
  std::string orient;
  std::string renderOrder;

  // This is used for loading multiple maps that share the same tiled set;
  std::map<GIDManager::Range, GIDManager::Range> tiledToEngineGID;

  // Map Properties
  std::string bgColorString;
  unsigned int tilesWide, tilesHigh, sizeOfTileWidth, sizeOfTileHeight;
  attributes["backgroundcolor"] = XML_Attribute("string", &bgColorString);
  attributes["width"] = XML_Attribute("unsigned int", &tilesWide);
  attributes["height"] = XML_Attribute("unsigned int", &tilesHigh);
  attributes["orientation"] = XML_Attribute("string", &orient);
  attributes["renderorder"] = XML_Attribute("string", &renderOrder);
  attributes["tilewidth"] = XML_Attribute("unsigned int", &sizeOfTileWidth);
  attributes["tileheight"] = XML_Attribute("unsigned int", &sizeOfTileHeight);
  TMXLoadAttributes(node, attributes);

  auto tiledData = std::make_unique<TiledData>(tilesWide, tilesHigh);
  Vec3 light(1.0f, 1.0f, 1.0f);
  tiledData->SetAmbientLight(light);

  if ((sizeOfTileWidth != LENGINE_DEF_TILE_W) or
      (sizeOfTileHeight != LENGINE_DEF_TILE_H)) {
    std::stringstream ss;
    ss << "Couldn't load map named: " << TMXname
       << "\n    Tile width and height are not 16 pixels";
    LOG_FATAL(ss.str());
    throw ExitException();
  }

  if (orient != "orthogonal") {
    std::stringstream ss;
    ss << "Couldn't load map with orientation '" << orient
       << "'\n    orientation must be orthogonal";
    LOG_FATAL(ss.str());
    throw ExitException();
  }
  if (renderOrder != "left-up") {
    std::stringstream ss;
    ss << "Couldn't load map with renderorder '" << renderOrder
       << "'\n    orientation must be left-up";
    LOG_FATAL(ss.str());
    throw ExitException();
  }

  // Translate background color from string to int
  // const char* pointer=valueString.c_str();
  // pointer++; //Skip over the "#" character
  // tiledData->bgColor  =strtol( pointer, NULL, 16 );//base 16
  // pointer=NULL;

  node = node->first_node();  // point to the first child of <map>

  // Main loop, gets all of <map> children
  for (; node != 0; node = node->next_sibling()) {
    std::string nn(node->name());
    std::stringstream ss;
    ss << "TiledData::LoadResourceFromTMX() Currently reading a " << nn;
    // LOG_TRACE(ss.str());

    if (nn == "properties") {  // Map properties; only one of these in the whole
                               // map file; specifies the global script and what
                               // entities it listens to
      std::string listenString = "";
      std::string scriptString = "";
      double ambientR = 1.0f;
      double ambientG = 1.0f;
      double ambientB = 1.0f;

      TMXLoadProperties(node, tiledData->properties);

      attributes.clear();
      // attributes["SCRIPT"] = XML_Attribute("string", &scriptString);
      // attributes["LISTEN"] = XML_Attribute("string", &listenString);
      attributes["_AMBIENT_R"] = XML_Attribute("double", &ambientR);
      attributes["_AMBIENT_G"] = XML_Attribute("double", &ambientG);
      attributes["_AMBIENT_B"] = XML_Attribute("double", &ambientB);
      TMXLoadAttributesFromProperties(&tiledData->properties, attributes);
      Vec3 light(ambientR, ambientG, ambientB);
      tiledData->SetAmbientLight(light);

      if (scriptString == "") {
        continue;
      }

    }

    else if (nn == "tileset") {
      GID tilesetFirstGID;
      std::string tileSetSource =
          "";  // If the tileset is an external file, store its path here

      attributes.clear();
      attributes["firstgid"] = XML_Attribute("GID", &tilesetFirstGID);
      attributes["source"] = XML_Attribute("string", &tileSetSource);
      TMXLoadAttributes(node, attributes);

      // If there is a 'source' attribute defined in the tileset node, then that
      // means the tile set is an external file
      bool externalTilesetFile = (tileSetSource != "");
      xml_node<> *tileSetRootNode = node;
      xml_document<> tileSetDoc;
      std::unique_ptr<FileData> file;
      std::string xmlFile;
      if (externalTilesetFile) {
        try {
          // Some where in here memory may not be allocated properly, leading
          // the the node name corruption in the tileset loader
          std::stringstream externalTilesetFilePath;
          externalTilesetFilePath << "/Resources/Maps/" << tileSetSource;

          file.reset(LoadGenericFile(externalTilesetFilePath.str()).release());

          if (file.get()->length == 0) {
            LOG_INFO("File Length is null");
            tileSetRootNode = NULL;
          }
          if (file.get()->GetData() == NULL) {
            LOG_INFO("Data is null");
            tileSetRootNode = NULL;
          }

          xmlFile = std::string(file->GetData(), file->length);
          tileSetDoc.parse<0>((char *)xmlFile.c_str());

          tileSetRootNode = tileSetDoc.first_node("tileset");
        } catch (rapidxml::parse_error e) {
          std::stringstream ss;
          ss << "[C++; RSC_MapImpl::LoadTMX] TileSetDoc threw a "
                "rapidxml::parse error"
             << "\n    What is: " << e.what();
          LOG_INFO(ss.str());
          tileSetRootNode = NULL;

        } catch (LEngineFileException e) {
          std::stringstream ss;
          ss << "[C++; RSC_MapImpl::LoadTMX] TileSetDoc threw a fileException "
                "error"
             << "\n    What is: " << e.what();
          LOG_INFO(ss.str());
          tileSetRootNode = NULL;
        }
      }
      // External Tilesets don't have their first gid included in their file,
      // only the actual map assigns gids
      // that's why the first gid is passed as a separate value here
      if (tileSetRootNode != NULL) {
        tiledData->AddTileSet(
            TMXLoadTiledSet(tileSetRootNode, tilesetFirstGID, tiledData->gid));
      }
    }

    else if (nn == "layer") {
      tiledData->AddLayer(TMXLoadTiledTileLayer(node, tiledData->gid));
    }

    else if (nn == "objectgroup") {
      tiledData->AddLayer(TMXLoadTiledObjectLayer(node, tiledData.get()));
    }

    else if (nn == "imagelayer") {
      tiledData->AddLayer(
          TMXLoadTiledImageLayer(node, tiledData->gid, tilesWide, tilesHigh));
    }
  }

  // Iterate through every tile in every tile layer, updating its range
  GID oldID = 0;
  const GIDManager::Range *engineRange, *localRange;
  auto layerIt = tiledData->tiledTileLayers.begin();
  for (; layerIt != tiledData->tiledTileLayers.end(); layerIt++) {
    auto itX = (layerIt->get())->data2D.begin();
    for (; itX != (layerIt->get())->data2D.end(); itX++) {
      auto itY = itX->begin();
      for (; itY != itX->end(); itY++) {
        // Get the old GID that the tile expects
        oldID = (*itY);
        if (oldID == 0) {
          continue;
        }

        // Get the corresponding GID that the engine has
        auto i = tiledToEngineGID.begin();
        engineRange = NULL;
        localRange = NULL;

        for (; i != tiledToEngineGID.end(); i++) {
          // if the oldId is foudn within this old range.
          if ((i->first.low <= oldID) and (i->first.high >= oldID)) {
            engineRange = &(i->second);
            localRange = &(i->first);
            break;
          }
        }
        if (engineRange == NULL) {
          // LOG_INFO("Oh Noes");
          break;
        }

        GID localRangeDifference = (oldID - localRange->low);
        (*itY) = engineRange->low + localRangeDifference;
      }
    }
  }
  // GIDManager::SetHighestGID(finalGID);
  return tiledData;
}

std::unique_ptr<TiledTileLayer> TiledData::TMXLoadTiledTileLayer(
    rapidxml::xml_node<> *rootNode, const GIDManager &gidManager) {
  unsigned int animationRate = 0;  // if this variable stays zero, then there
                                   // won't be any tile animations
  int depth;
  unsigned int width;
  unsigned int height;
  std::string name;
  float alpha = 1.0f;

  std::string testString, valueString;

  XML_AttributeMap attributes;

  attributes.clear();
  attributes["name"] = XML_Attribute("string", &name);
  attributes["width"] = XML_Attribute("unsigned int", &width);
  attributes["height"] = XML_Attribute("unsigned int", &height);
  attributes["opacity"] = XML_Attribute("float", &alpha);
  TMXLoadAttributes(rootNode, attributes);

  rapidxml::xml_node<> *propertiesNode = rootNode->first_node("properties");

  bool propertyCollision = false;
  bool propertyHMap = false;
  std::map<std::string, std::string> extraProperties;
  std::string vertShaderName;
  std::string fragShaderName;
  std::string geoShaderName;

  XML_PropertyMap properties;
  if (propertiesNode != NULL) {
    TMXLoadProperties(propertiesNode, properties);

    attributes.clear();
    attributes[tiledProperties::DEPTH] = XML_Attribute("int", &depth);
    attributes[tiledProperties::tile::SOLID] =
        XML_Attribute("bool", &propertyCollision);
    attributes[tiledProperties::tile::HMAP] =
        XML_Attribute("bool", &propertyHMap);
    attributes[tiledProperties::SHADER_FRAG] =
        XML_Attribute("string", &fragShaderName);
    attributes[tiledProperties::SHADER_VERT] =
        XML_Attribute("string", &vertShaderName);
    attributes[tiledProperties::SHADER_GEO] =
        XML_Attribute("string", &geoShaderName);
    attributes[tiledProperties::tile::ANIMATION_SPEED] =
        XML_Attribute("unsigned int", &animationRate);
    TMXLoadAttributesFromProperties(
        &properties,
        attributes);  // store unspecified properties
                      // in tileLayer->extraproperties
  }

  std::unique_ptr<TiledTileLayer> tileLayer(
      new TiledTileLayer(width, height, name, depth, &gidManager));

  tileLayer->mShaderFrag = fragShaderName;
  tileLayer->mShaderVert = vertShaderName;
  tileLayer->mShaderGeo = geoShaderName;

  tileLayer->layerFlags = 0;
  tileLayer->layerOpacity = alpha;
  CopyPropertyMap(properties, tileLayer->properties);

  if (propertyCollision) {
    tileLayer->layerFlags = tileLayer->layerFlags | TF_solid;
  }
  if (propertyHMap) {
    tileLayer->layerFlags = tileLayer->layerFlags | TF_useHMap;
  }

  auto dataNode = rootNode->first_node("data");
  auto encoding = dataNode->first_attribute("encoding");
  auto isInfinite = (dataNode->first_node("chunk") != NULL);
  if (isInfinite) {
    LOG_FATAL("Chunks / Infinite maps are not supported")
    throw ExitException();
  }
  if (encoding != NULL) {
    auto encodingStr = encoding->value();
    bool supported = false;
    for (auto str = TiledTileLayer::SUPPORTED_ENCODINGS.begin();
         str != TiledTileLayer::SUPPORTED_ENCODINGS.end(); str++) {
      if (*str == encodingStr) {
        supported = true;
      }
    }

    if (!supported) {
      std::stringstream ss;
      ss << "Tiled Data format '" << encodingStr << "' not supported";
      LOG_FATAL(ss.str());
      throw ExitException();
    }
  }
  std::vector<GID> data;
  GID id = 0;
  for (auto subnode = dataNode->first_node(); subnode;
       subnode = subnode->next_sibling()) {
    auto gidNode = subnode->first_attribute("gid");
    if (gidNode != NULL) {
      valueString = gidNode->value();
      id = strtol(valueString.c_str(), NULL, 10);
    } else {
      id = 0;
    }

    data.push_back(id);
  }
  unsigned int tilesWide = tileLayer->tileWidth;

  // load the correct data into 2dmap
  unsigned int x = 0;
  unsigned int y = 0;

  // Want to figure out what tileset this layer uses
  // All tilesets should be loaded by this point
  // This function will simply select the first tile set that it sees.
  tileLayer->tileSet = NULL;
  for (unsigned int i = 0; i < data.size(); i++) {
    if (tileLayer->tileSet == NULL) {
      if (data[i] != 0) {
        auto tileSet = ((TiledSet *)gidManager.GetItem(data[i]));
        tileLayer->tileSet = tileSet;
      }
    }
    tileLayer->data2D[x][y] = data[i];
    x++;
    if (x >= tilesWide) {
      x = 0;
      y++;
    }
  }
  if (tileLayer->tileSet == NULL) {
    std::stringstream ss;
    ss << "TiledLayer named " << name << " doesn't have a tileSet";
    LOG_ERROR(ss.str());
    return NULL;
  }

  return tileLayer;
}

std::unique_ptr<TiledObjectLayer> TiledData::TMXLoadTiledObjectLayer(
    rapidxml::xml_node<> *rootNode, TiledData *tiledData) {
  XML_AttributeMap attributes;
  std::string objectLayerName = rootNode->first_attribute()->value();

  /// \TODO what if the layer HAS NO ATTRIBUTES?

  // Object Layer properties
  rapidxml::xml_node<> *subnodeProperties = rootNode->first_node();
  XML_PropertyMap properties;
  // Load properties into layer
  TMXLoadProperties(subnodeProperties, properties);

  // Extract expected attributes
  int depth = 0;
  attributes.clear();
  attributes["DEPTH"] = XML_Attribute("int", &depth);
  TMXLoadAttributesFromProperties(&properties, attributes);

  std::unique_ptr<TiledObjectLayer> objectLayer =
      std::make_unique<TiledObjectLayer>(0, 0, objectLayerName, depth,
                                         &tiledData->gid);
  objectLayer->properties = properties;

  // Siblings of <properties> are actual objects
  for (auto subnodeObject = subnodeProperties->next_sibling();
       subnodeObject != 0; subnodeObject = subnodeObject->next_sibling()) {
    int objID, objX, objY, objWidth, objHeight;
    std::string objName, objPrefab;

    attributes.clear();
    attributes["id"] = XML_Attribute("int", &objID);
    attributes["x"] = XML_Attribute("int", &objX);
    attributes["y"] = XML_Attribute("int", &objY);
    attributes["width"] = XML_Attribute("int", &objWidth);
    attributes["height"] = XML_Attribute("int", &objHeight);
    attributes["type"] = XML_Attribute("string", &objPrefab);
    attributes["name"] = XML_Attribute("string", &objName);
    TMXLoadAttributes(subnodeObject, attributes);

    // Convert bottom-left origin to top-left origin
    objY -= objHeight;

    // create new object
    objectLayer->objects[objID] = TiledObject();
    TiledObject &newObj = (objectLayer->objects[objID]);

    // assign values
    // newObj.scripts = "";
    newObj.name = objName;
    newObj.prefabName = objPrefab;
    newObj.extraData = NULL;
    newObj.x = objX;
    newObj.y = objY;
    newObj.w = objWidth;
    newObj.h = objHeight;
    newObj.tiledID = objID;

    // Get pointer to node containing the new object's properties
    rapidxml::xml_node<> *objectPropertiesNode =
        subnodeObject->first_node("properties");

    if (objectPropertiesNode != NULL) {
      std::string name;
      std::string value;
      std::string testString, valueString;

      // Variables to store property data that needs processed
      std::string eventString = "";
      std::string listenString = "";
      std::string type = "";
      newObj.useEntrance = false;
      newObj.parent = 0;

      /// \TODO use TMX helper funcitons
      /// \TODO Remove Exits
      // User defined properties for this particular object
      // Going to read this raw, without TMX hepler functions
      // more efficent, more contorl.
      rapidxml::xml_node<> *objectPropertyNode =
          objectPropertiesNode->first_node();
      for (; objectPropertyNode != 0;
           objectPropertyNode = objectPropertyNode->next_sibling()) {
        type = "string";

        // Get name, value, type
        for (rapidxml::xml_attribute<> *attr =
                 objectPropertyNode->first_attribute();
             attr; attr = attr->next_attribute()) {
          testString = attr->name();
          valueString = attr->value();

          if (testString == "name") {
            name = valueString;
          } else if (testString == "type") {
            type = valueString;
          } else if (testString == "value") {
            value = valueString;
          }
        }

        // Assign property to correct variable
        if (name == tiledProperties::object::SCRIPT) {
          newObj.scripts = value;
        }
        /*
        if (name == "LIGHT") {
          if (valueString == "true") {
            newObj.light = true;
          } else {
            newObj.light = false;
          }
        }
        else if (name == tiledProperties::object::ENTRANCE_ID) {
          eventNum = strtol(value.c_str(), NULL, 10);
        }
        */

        else if (name == "MAP") {
          eventString = value;
        } else if (name == tiledProperties::object::USE_ENTRANCE) {
          if (valueString == "true") {
            newObj.useEntrance = true;
          }
        } else if (name == tiledProperties::object::LISTEN_ID) {
          listenString = value;
        } else if (name == tiledProperties::object::PARENT) {
          newObj.parent = strtol(value.c_str(), NULL, 10);
        }

        // If not what the engine expected, put in correct container
        else {
          if (type == "string") {
            newObj.properties.strings[name] = value;
          } else if (type == "bool") {
            newObj.properties.bools[name] = (value == "true");
          } else if (type == "int") {
            newObj.properties.ints[name] = strtol(value.c_str(), NULL, 10);
          } else if (type == "float") {
            newObj.properties.floats[name] = atof(value.c_str());
          }
        }
      }  // End for loop, go to next sibling (Next property)

      TMXProcessEventListeners(listenString, newObj.eventSources);
    }
  }
  return objectLayer;
}

std::unique_ptr<TiledImageLayer> TiledData::TMXLoadTiledImageLayer(
    rapidxml::xml_node<> *rootNode, const GIDManager &gidManager, int mapTilesW,
    int mapTilesH) {
  rapidxml::xml_node<> *subNodeImage = rootNode->first_node();

  std::string name;
  XML_AttributeMap attributes;
  attributes["name"] = XML_Attribute("string", &name);
  TMXLoadAttributes(rootNode, attributes);

  std::string texturePath;
  int w, h;
  float alpha = 1.0f;
  attributes.clear();
  attributes["source"] = XML_Attribute("string", &texturePath);
  attributes["width"] = XML_Attribute("int", &w);
  attributes["height"] = XML_Attribute("int", &h);
  attributes["opacity"] = XML_Attribute("float", &alpha);
  TMXLoadAttributes(subNodeImage, attributes);

  rapidxml::xml_node<> *subNodeProperties = subNodeImage->next_sibling();
  int depth = 0;

  const RSC_Texture *texture = K_TextureMan.GetItem(texturePath);
  if (texture == NULL) {
    K_TextureMan.LoadItem(texturePath, texturePath);
    texture = K_TextureMan.GetItem(texturePath);
    if (texture == NULL) {
      std::stringstream ss;
      ss << "Couldn't load texture for Image Layer " << texturePath;
      LOG_INFO(ss.str());
      return NULL;
    }
  }

  float paralaxX, paralaxY;
  paralaxX = 1.0f;
  paralaxY = 1.0f;

  bool repeatX = false;
  bool repeatY = false;
  bool stretchX = false;
  bool stretchY = false;

  XML_PropertyMap properties;
  attributes.clear();
  attributes[tiledProperties::image::PARALLAX_X] =
      XML_Attribute("float", &paralaxX);
  attributes[tiledProperties::image::PARALLAX_Y] =
      XML_Attribute("float", &paralaxY);
  attributes[tiledProperties::image::STRETCH_X] =
      XML_Attribute("bool", &stretchX);
  attributes[tiledProperties::image::STRETCH_Y] =
      XML_Attribute("bool", &stretchY);
  attributes[tiledProperties::image::REPEAT_X] =
      XML_Attribute("bool", &repeatX);
  attributes[tiledProperties::image::REPEAT_Y] =
      XML_Attribute("bool", &repeatY);
  attributes[tiledProperties::DEPTH] = XML_Attribute("int", &depth);
  TMXLoadProperties(subNodeProperties, properties);
  TMXLoadAttributesFromProperties(&properties, attributes);

  std::unique_ptr<TiledImageLayer> imageLayer(new TiledImageLayer(
      mapTilesW, mapTilesH, name, depth, &gidManager, texture));
  imageLayer->SetParallax(Vec2(paralaxX, paralaxY));
  imageLayer->SetAlpha(alpha);
  imageLayer->SetRepeatX(repeatX);
  imageLayer->SetRepeatY(repeatY);
  imageLayer->SetStretchToMapX(stretchX);
  imageLayer->SetStretchToMapY(stretchY);
  imageLayer->properties = properties;

  return imageLayer;
}

std::unique_ptr<TiledSet> TiledData::TMXLoadTiledSet(
    rapidxml::xml_node<> *tiledSetRootNode, const GID &firstGID,
    GIDManager &gidManager) {
  auto usesGrid = (tiledSetRootNode->first_node("grid") != NULL);
  if (usesGrid) {
    LOG_ERROR("TiledSets with multiple images are not supported")
    return NULL;
  }
  auto subNodeImage = tiledSetRootNode->first_node("image");
  // Tileset attributes
  GID tilesetFirstGID = firstGID;
  std::string name = "";

  unsigned int tileWidth;
  unsigned int tileHeight;

  XML_AttributeMap attributes;
  attributes["name"] = XML_Attribute("string", &name);
  attributes["tilewidth"] = XML_Attribute("unsigned int", &tileWidth);
  attributes["tileheight"] = XML_Attribute("unsigned int", &tileHeight);
  TMXLoadAttributes(tiledSetRootNode, attributes);

  // IMAGE XML_Attributes
  std::string transparentColor = "ff00ff";
  std::string textureName;

  attributes.clear();
  attributes["source"] = XML_Attribute("string", &textureName);
  attributes["trans"] = XML_Attribute("string", &transparentColor);
  TMXLoadAttributes(subNodeImage, attributes);

  // Load tileset into engine and set firstGID
  auto returnSmartPonter = std::unique_ptr<TiledSet>(new TiledSet(
      name, textureName, tileWidth, tileHeight, tilesetFirstGID, &gidManager));

  TiledSet *ts = returnSmartPonter.get();
  ts->transparentColor = transparentColor;

  // Set up Tiled GID Range
  ts->SetFirstGID(tilesetFirstGID);
  ts->SetLastGID((ts->GetFirstGID() + ts->GetTilesTotal()) - 1);

  // Get Unique Tile Properties
  auto subNodeTileRoot = tiledSetRootNode->first_node("tile");

  // Iterate through all of the <tile> tags
  GID tilePropertyID;
  std::string valueString;
  for (; subNodeTileRoot != 0;
       subNodeTileRoot = subNodeTileRoot->next_sibling()) {
    // Get GID of tile that the properties are associated with
    // This GID is the value of the tile within the tileset, between 0 and (max
    // number of tiles -1)
    valueString = subNodeTileRoot->first_attribute("id")->value();
    tilePropertyID = strtol(valueString.c_str(), NULL, 10);

    // Add tileset's starting GID to get the actual value of the tile.
    tilePropertyID += tilesetFirstGID;

    auto tileProperties = subNodeTileRoot->first_node("properties");

    if (tileProperties != NULL) {
      XML_PropertyMap properties;
      TMXLoadProperties(tileProperties, properties);
      ts->tileProperties[tilePropertyID] = properties;
    }

    auto animationsNode = subNodeTileRoot->first_node("animation");
    if (animationsNode != NULL) {
      auto frameNode = animationsNode->first_node("frame");
      TileAnimation animation;
      for (; frameNode != 0; frameNode = frameNode->next_sibling()) {
        TileAnimation::Frame f;
        f.tileID =
            StringToNumber<int>(frameNode->first_attribute("tileid")->value());
        f.tileID += tilesetFirstGID;
        float length = StringToNumber<int>(
            frameNode->first_attribute("duration")->value());
        length = (length / 1000) * 60;
        f.length = int(length);
        animation.frames.push_back(f);
      }
      returnSmartPonter->AddTileAnimation(tilePropertyID, animation);
    }
  }

  return returnSmartPonter;
}

void RSC_MapImpl::DeleteLayer(TiledLayerGeneric *layer) {
  tiledData->DeleteLayer(layer);
}

void RSC_MapImpl::SetAmbientLight(Vec3 light) {
  tiledData->SetAmbientLight(light);
}

Vec3 RSC_MapImpl::GetAmbientLight() { return tiledData->GetAmbientLight(); }

void RSC_Map::ExposeLuaInterface(lua_State *state) {
  luabridge::getGlobalNamespace(state)
      .beginNamespace("CPP")  //'CPP' table

      .beginClass<TiledLayerGeneric>("TiledLayerGeneric")
      .addFunction("SetAlpha", &TiledLayerGeneric::SetAlpha)
      .addFunction("GetAlpha", &TiledLayerGeneric::GetAlpha)
      .addFunction("GetFlags", &TiledLayerGeneric::GetFlags)
      .endClass()

      .deriveClass<TiledTileLayer, TiledLayerGeneric>("TiledTileLayer")
      .addFunction("GetTileProperty", &TiledTileLayer::GetTileProperty)
      .addFunction("UsesHMaps", &TiledTileLayer::UsesHMaps)
      .addFunction("GetTile", &TiledTileLayer::GetTile)
      .addFunction("SetTile", &TiledTileLayer::SetTile)
      .addFunction("HasTile", &TiledTileLayer::HasTile)
      .addFunction("UpdateRenderArea", &TiledTileLayer::UpdateRenderArea)
      .endClass()

      .beginClass<RSC_Map>("RSC_Map")
      .addFunction("GetTileLayer", &RSC_Map::GetTileLayer)
      .addFunction("DeleteLayer", &RSC_Map::DeleteLayer)
      .addFunction("GetAmbientLight", &RSC_Map::GetAmbientLight)
      .addFunction("GetSolidTileLayers", &RSC_Map::GetSolidTileLayers)
      .addFunction("GetProperty", &RSC_Map::GetProperty)
      .addFunction("GetWidthTiles", &RSC_Map::GetWidthTiles)
      .addFunction("GetHeightTiles", &RSC_Map::GetHeightTiles)
      .addFunction("GetWidthPixels", &RSC_Map::GetWidthPixels)
      .addFunction("GetHeightPixels", &RSC_Map::GetHeightPixels)
      .endClass()

      .endNamespace();
}
