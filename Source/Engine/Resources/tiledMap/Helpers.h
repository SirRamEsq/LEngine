#ifndef LENGINE_TILED_HELPERS
#define LENGINE_TILED_HELPERS

#include "../../Coordinates.h"
#include "../../Defines.h"

#include <map>
#include <string>
#include <rapidxml.hpp>

enum L_TILED_LAYER_TYPE {
  LAYER_GENERIC = 0,
  LAYER_OBJECT = 1,
  LAYER_IMAGE = 2,
  LAYER_TILE = 3
};

const std::string L_TILED_LAYER_TYPE_STRINGS[] = {
    "LAYER_GENERIC", "LAYER_OBJECT", "LAYER_IMAGE", "LAYER_TILE"};

enum L_TILED_IMAGE_LAYER_FLAGS {
  IMG_LAYER_REPEAT_X = 1,
  IMG_LAYER_REPEAT_Y = 2
};

// These Types are used as helpers in the TMX loading functions
// XML_Attributes have a name and reference a variable to store data in
// They do not contain type information, you are expected to know what type they
// are
// and provide storage for their values
// These are used for built in variables in Tiled, the types of which are
// already known.
// Type, Value
typedef std::pair<std::string, void *> XML_Attribute;
//<Name                  <Type, value> >
typedef std::map<std::string, XML_Attribute> XML_AttributeMap;

// Properties are made of three strings, a name, a type, and a value in string
// form
// Properties are used when you have no idea what data the user may send via
// Tiled
// Type      Value
typedef std::pair<std::string, std::string> StringPair;
// Name
typedef std::map<std::string, StringPair> XML_PropertyMap;

// insert these into a 'tiled' namespace
void CopyPropertyMap(const XML_PropertyMap &source,
                     XML_PropertyMap &destination);

/*TERMINOLOGY
A Map Entrance is a ID with a position
An Exit is an Entrance with a map name string (even if the string is blank;
referring to the previous map)
When you collide with an exit, you go to a new map
You don't collide with entrances
An entrance can only be used to enter the map
An Exit can only be used to exit the map
*/

struct MapEntrance {
  unsigned int mEntranceID;
  Rect mPosition;
};

struct MapExit {
  unsigned int mEntranceID;
  Rect mPosition;
  std::string mMapName;
};

/// This class encapsulates a tiled map entity's properties and is sorted by
/// type
struct TiledProperties {
  TiledProperties(const TiledProperties *rhs);
  TiledProperties(const XML_PropertyMap *properties);
  TiledProperties();
  std::map<std::string, int> ints;
  std::map<std::string, bool> bools;
  std::map<std::string, float> floats;
  std::map<std::string, std::string> strings;
};

// Give the function a string, its type, and where to store the data
void TMXProcessType(std::string &type, std::string &value, void *data);

// for use with an empty property map, stores all properties found with type
// information
void TMXLoadProperties(rapidxml::xml_node<> *rootPropertyNode,
                       XML_PropertyMap &properties);

// Will insert data from the property map into the data pointed by the
// attribute map
void TMXLoadAttributesFromProperties(const XML_PropertyMap *properties,
                                     XML_AttributeMap &attributes);

// For use with either an empty or populated attribute map
void TMXLoadAttributes(rapidxml::xml_node<> *rootAttributeNode,
                       XML_AttributeMap &attributes);
void TMXProcessEventListeners(std::string &listenersString,
                              std::vector<EID> &listeners);

#endif
