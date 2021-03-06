#include "../catch.hpp"
#include "../utility.h"
#include "../../Engine/GameSave.h"

TEST_CASE("Can Save and Load GameSaves", "[gameSave]") {
  InitTestPhysfs();

  std::string saveName("UNIT_TEST_SAVE");
  GameSave saveFile(saveName);

  std::string boolName = "boolTrue";
  std::string intName = "negativeInt";
  std::string doubleName = "negativeDouble";
  std::string stringName = "key";

  bool boolValue = true;
  int intValue = -234;
  double doubleValue = -2345.99833f;
  std::string stringValue = "strValue!324";

  saveFile.SetBool(boolName, boolValue);
  saveFile.SetInt(intName, intValue);
  saveFile.SetDouble(doubleName, doubleValue);
  saveFile.SetString(stringName, stringValue);

  REQUIRE(saveFile.GetBool(boolName) == boolValue);
  REQUIRE(saveFile.GetInt(intName) == intValue);
  REQUIRE(saveFile.GetDouble(doubleName) == doubleValue);
  REQUIRE(saveFile.GetString(stringName) == stringValue);

  REQUIRE(saveFile.ExistsBool(boolName) == true);
  REQUIRE(saveFile.ExistsInt(intName) == true);
  REQUIRE(saveFile.ExistsDouble(doubleName) == true);
  REQUIRE(saveFile.ExistsString(stringName) == true);

  std::string nonsense = "asfkb";
  REQUIRE(saveFile.ExistsBool(nonsense) == false);
  REQUIRE(saveFile.ExistsInt(nonsense) == false);
  REQUIRE(saveFile.ExistsDouble(nonsense) == false);
  REQUIRE(saveFile.ExistsString(nonsense) == false);

  REQUIRE(saveFile.FileExists() == false);
  saveFile.WriteToFile();

  GameSave newSaveFile(saveName);
  REQUIRE(newSaveFile.FileExists() == true);
  newSaveFile.ReadFromFile();

  REQUIRE(newSaveFile.GetBool(boolName) == boolValue);
  REQUIRE(newSaveFile.GetInt(intName) == intValue);
  REQUIRE(newSaveFile.GetDouble(doubleName) == doubleValue);
  REQUIRE(newSaveFile.GetString(stringName) == stringValue);

  newSaveFile.DeleteFile();
  REQUIRE(newSaveFile.FileExists() == false);

  CloseTestPhysfs();
}
