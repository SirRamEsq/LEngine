#ifndef LENGINE_RSC_PREFAB
#define LENGINE_RSC_PREFAB

#include <memory>
#include <vector>

#include "tiledMap/Helpers.h"

class RSC_Prefab {
  typedef std::vector<std::string> ScriptNames;

 public:
  RSC_Prefab(const ScriptNames scripts, const TiledProperties* properties);

  const ScriptNames mScripts;
  const TiledProperties mProperties;
  static std::unique_ptr<RSC_Prefab> LoadResource(const std::string& fileName);
  static std::unique_ptr<RSC_Prefab> LoadFromXML(const char *dat, unsigned int size);
};

#endif
