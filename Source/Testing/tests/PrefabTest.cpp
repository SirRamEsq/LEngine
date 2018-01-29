#include "../catch.hpp"
#include "../utility.h"
#include "../../Engine/Resources/RSC_Prefab.h"
#include "../../Engine/GenericContainer.h"

TEST_CASE("Can Load Prefab from XML", "[resources][prefab]") {
  InitTestPhysfs();
  // GenericContainer<RSC_Prefab> prefabs;
  auto prefab1 = RSC_Prefab::LoadResource("System/cppTestPrefab.xml");

  // Ensure data structures have correct number of elements loaded
  REQUIRE(prefab1.get() != NULL);
  REQUIRE(prefab1->mScripts.size() == 3);
  REQUIRE(prefab1->mProperties.ints.size() == 1);
  REQUIRE(prefab1->mProperties.bools.size() == 3);
  REQUIRE(prefab1->mProperties.floats.size() == 1);
  REQUIRE(prefab1->mProperties.strings.size() == 2);

  // Scripts expected
  std::string script1 = "Common/entityCollision.lua";
  std::string script2 = "Common/tileCollision.lua";
  std::string script3 = "mainScript.lua";

  REQUIRE(prefab1->mScripts[0] == script1);
  REQUIRE(prefab1->mScripts[1] == script2);
  REQUIRE(prefab1->mScripts[2] == script3);

  // Data Expected
  auto string1 = std::pair<std::string, std::string>("YADDA", "ADDAY");
  auto string2 = std::pair<std::string, std::string>("YADDA2", "2ADDAY");

  auto bool1 = std::pair<std::string, bool>("truthy", true);
  auto bool2 = std::pair<std::string, bool>("truthy2", false);
  auto bool3 = std::pair<std::string, bool>("truthy3", true);

  auto int1 = std::pair<std::string, int>("point", 21);

  auto float1 = std::pair<std::string, float>("decimal", 324.25);

  std::string key;
  std::string strValue1;
  std::string strValue2;

  key = std::get<0>(string1);
  strValue1 = std::get<1>(string1);
  strValue2 = (prefab1->mProperties.strings.find(key))->second;
  REQUIRE(strValue1 == strValue2);

  key = std::get<0>(string2);
  strValue1 = std::get<1>(string2);
  strValue2 = (prefab1->mProperties.strings.find(key))->second;
  REQUIRE(strValue1 == strValue2);

  bool boolValue1;
  bool boolValue2;

  key = std::get<0>(bool1);
  boolValue1 = std::get<1>(bool1);
  boolValue2 = (prefab1->mProperties.bools.find(key))->second;
  REQUIRE(boolValue1 == boolValue2);

  key = std::get<0>(bool2);
  boolValue1 = std::get<1>(bool2);
  boolValue2 = (prefab1->mProperties.bools.find(key))->second;
  REQUIRE(boolValue1 == boolValue2);

  key = std::get<0>(bool3);
  boolValue1 = std::get<1>(bool3);
  boolValue2 = (prefab1->mProperties.bools.find(key))->second;
  REQUIRE(boolValue1 == boolValue2);

  int intValue1;
  int intValue2;
  key = std::get<0>(int1);
  intValue1 = std::get<1>(int1);
  intValue2 = (prefab1->mProperties.ints.find(key))->second;
  REQUIRE(intValue1 == intValue2);

  float floatValue1;
  float floatValue2;
  key = std::get<0>(float1);
  floatValue1 = std::get<1>(float1);
  floatValue2 = (prefab1->mProperties.floats.find(key))->second;
  REQUIRE(floatValue1 == floatValue2);

  CloseTestPhysfs();
}
