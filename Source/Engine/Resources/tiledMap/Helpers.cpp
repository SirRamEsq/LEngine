#include "Helpers.h"

void CopyPropertyMap(const PropertyMap &source, PropertyMap &destination) {
  destination.clear();
  for (auto i = source.begin(); i != source.end(); i++) {
    destination[i->first] = i->second;
  }
}
