#include "RSC_Prefab.h"

#include <rapidxml.hpp>

#include "ResourceLoading.h"
#include "../Errorlog.h"
#include "../Defines.h"

RSC_Prefab::RSC_Prefab(const std::vector<std::string> scripts,
                       const TiledMapProperties *properties)
    : mScripts(scripts), mProperties(properties) {}

std::unique_ptr<RSC_Prefab> RSC_Prefab::LoadFromXML(const char *dat,
                                                    unsigned int size) {
  std::vector<std::string> finalScripts;
  TiledMapProperties finalProperties;
  std::string XML = std::string(dat, size);

  using namespace rapidxml;
  xml_document<> doc;  // character type defaults to char
  doc.parse<0>((char *)(XML.c_str()));

  // Find Specific Node
  xml_node<> *node = doc.first_node("prefab");
  xml_node<> *nodeScripts = node->first_node("scripts");
  xml_node<> *nodeProperties = node->first_node("properties");

  if (nodeProperties != NULL) {
    PropertyMap properties;
    TMXLoadProperties(nodeProperties, properties);
    finalProperties = TiledMapProperties(&properties);
  }
  if (nodeScripts != NULL) {
    for (auto script = nodeScripts->first_node("script"); script != NULL;
         script = script->next_sibling()) {
      auto scriptName = script->first_attribute("name")->value();
      finalScripts.push_back(scriptName);
    }
  }

  return std::make_unique<RSC_Prefab>(finalScripts, &finalProperties);
}

std::unique_ptr<RSC_Prefab> RSC_Prefab::LoadResource(
    const std::string &fileName) {
  std::unique_ptr<RSC_Prefab> prefab = NULL;
  try {
    std::string fullPath = "Resources/Prefabs/" + fileName;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      std::stringstream ss;
      ss << "Prefab " << fullPath << " couldn't be found.";
      LOG_ERROR(ss.str());
      return NULL;
    }
    prefab = LoadFromXML(data.get()->GetData(), data.get()->length);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }

  return prefab;
}
