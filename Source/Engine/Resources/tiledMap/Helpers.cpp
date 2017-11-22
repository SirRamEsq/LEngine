#include "Helpers.h"
#include "GID.h"
#include <string.h>
#include <sstream>

void CopyPropertyMap(const PropertyMap &source, PropertyMap &destination) {
  destination.clear();
  for (auto i = source.begin(); i != source.end(); i++) {
    destination[i->first] = i->second;
  }
}

TiledMapProperties::TiledMapProperties() {}
TiledMapProperties::TiledMapProperties(const TiledMapProperties *rhs) {
  if (rhs == NULL) {
    return;
  }
  for (auto i = rhs->ints.begin(); i != rhs->ints.end(); i++) {
    ints[i->first] = i->second;
  }
  for (auto i = rhs->bools.begin(); i != rhs->bools.end(); i++) {
    bools[i->first] = i->second;
  }
  for (auto i = rhs->floats.begin(); i != rhs->floats.end(); i++) {
    floats[i->first] = i->second;
  }
  for (auto i = rhs->strings.begin(); i != rhs->strings.end(); i++) {
    strings[i->first] = i->second;
  }
}

TiledMapProperties::TiledMapProperties(const PropertyMap *rhs) {
  for (auto i = rhs->begin(); i != rhs->end(); i++) {
	auto name = i->first;
	auto type = std::get<0>(i->second);
	auto value = std::get<1>(i->second);
    if (type == "string") {
      strings[name] = value;
    } else if (type == "bool") {
      bools[name] = (value == "true");
    } else if (type == "int") {
      ints[name] = strtol(value.c_str(), NULL, 10);
    } else if (type == "float") {
      floats[name] = atof(value.c_str());
    }
  }
}

void TMXLoadAttributesFromProperties(const PropertyMap *properties,
                                     AttributeMap &attributes) {
  std::string data;
  std::string type;
  std::string name;
  for (auto i = properties->begin(); i != properties->end(); i++) {
    name = i->first;
    auto attributeIterator = attributes.find(name);

    if (attributeIterator != attributes.end()) {
      type = attributeIterator->second.first;
      data = i->second.second;
      TMXProcessType(type, data, attributeIterator->second.second);
    }
  }
}
// root node passed should point to <properties> tag
void TMXLoadProperties(rapidxml::xml_node<> *rootPropertyNode,
                       PropertyMap &properties) {
  using namespace rapidxml;
  xml_node<> *propertyNode =
      rootPropertyNode->first_node();  // pointing to property

  std::string nameString, valueString, typeString;

  // properties are siblings, not attributes
  xml_attribute<> *attributeName;
  xml_attribute<> *attributeType;
  xml_attribute<> *attributeValue;
  for (; propertyNode; propertyNode = propertyNode->next_sibling()) {
    typeString = "string";
    nameString = "";
    valueString = "";
    attributeName = propertyNode->first_attribute("name");
    attributeType = propertyNode->first_attribute("type");
    attributeValue = propertyNode->first_attribute("value");

    if (attributeName) {
      nameString = attributeName->value();
    }
    if (attributeType) {
      typeString = attributeType->value();
    }
    if (attributeValue) {
      valueString = attributeValue->value();
    }

    properties[nameString] = StringPair(typeString, valueString);
  }
}

void TMXProcessEventListeners(std::string &listenersString,
                              std::vector<EID> &listeners) {
  if (listenersString != "") {
    std::vector<const char *> subStrings;
    char *tempString = NULL;
    char *propertyCString = new char[listenersString.size() + 1];
    strcpy(propertyCString, listenersString.c_str());

    tempString = strtok(propertyCString, ", ");
    if (tempString != NULL) {
      subStrings.push_back(tempString);
      while (true) {
        tempString = strtok(NULL, ", ");
        if (tempString == NULL) {
          break;
        }
        subStrings.push_back(tempString);
      }
    } else {
      subStrings.push_back(listenersString.c_str());
    }

    // Have list of strings, now just set up the correct listen IDs for the
    // object
    int tempID;

    std::vector<const char *>::iterator i = subStrings.begin();
    for (; i != subStrings.end(); i++) {
      tempID = strtol((*i), NULL, 10);

      listeners.push_back(tempID);
    }

    delete[] propertyCString;
  }
}

void TMXProcessType(std::string &type, std::string &value, void *data) {
  if (type == "string") {
    *((std::string *)(data)) = value;
  } else if (type == "int") {
    *((int *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "unsigned long") {
    *((unsigned long *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "unsigned int") {
    *((unsigned int *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "long") {
    *((long *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "EID") {
    *((EID *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "GID") {
    *((GID *)(data)) = strtol(value.c_str(), NULL, 10);
  } else if (type == "float") {
    *((float *)(data)) = atof(value.c_str());
  } else if (type == "double") {
    *((double *)(data)) = atof(value.c_str());
  } else if (type == "bool") {
    *((bool *)(data)) = (value == "true");
  }
}
// Pass a map with keys to get their respective values from the xml file
// Pass an empty map to get everything
void TMXLoadAttributes(rapidxml::xml_node<> *rootAttributeNode,
                       AttributeMap &attributes) {
  using namespace rapidxml;
  std::string key, value, type;
  AttributeMap::iterator i;
  bool emptyMap = attributes.empty();  // returns true if map is empty

  // Iterate through each attribute in the given node
  for (xml_attribute<> *attr = rootAttributeNode->first_attribute(); attr;
       attr = attr->next_attribute()) {
    key = attr->name();
    value = attr->value();
    // LOG_INFO("Key is ");
    // LOG_INFO(key);

    // If the attribute map passed in is empty, simply fill it with every
    // attribute found
    if (emptyMap) {
      attributes[key] = Attribute(value, NULL);
    }
    // If the attribute map is not empty, match each found key in the node with
    // it's corresponding value in the map
    else {
      i = attributes.find(key);
      if (i != attributes.end()) {
        type = i->second.first;
        TMXProcessType(type, value, i->second.second);
      }
    }
  }
}
