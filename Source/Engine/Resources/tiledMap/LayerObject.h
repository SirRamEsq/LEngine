#ifndef LENGINE_TILED_LAYER_OBJECT
#define LENGINE_TILED_LAYER_OBJECT

#include "LayerGeneric.h"

#include <map>
#include <vector>

// A TiledObject is simply a bag of data that is used to instantiate an entity
// when the map is loaded
struct TiledObject {
  std::string name, type, script;
  int x;
  int y;
  int w;
  int h;
  EID parent;
  bool useEntrance;
  bool light;
  void *extraData;

  EID tiledID;

  int flags;

  std::vector<EID> eventSources;

  std::map<std::string, int> intProperties;
  std::map<std::string, bool> boolProperties;
  std::map<std::string, float> floatProperties;
  std::map<std::string, std::string> stringProperties;
};

class TiledObjectLayer : public TiledLayerGeneric {
 public:
  TiledObjectLayer(const unsigned int &pixelW, const unsigned int &pixelH,
                   const std::string &name, const MAP_DEPTH &depth,
                   const GIDManager *g);
  TiledObjectLayer(const TiledObjectLayer &rhs, const GIDManager *g);

  std::map<EID, TiledObject> objects;
};

#endif
