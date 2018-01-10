#ifndef LENGINE_SAVELOAD
#define LENGINE_SAVELOAD

#include <string>
#include <unordered_map>
#include "LuaInclude.h"

#include "Resources/ResourceLoading.h"

class GameSave {
 public:
  GameSave(const std::string& name);

  /// Writes contents to file
  void WriteToFile();
  /// Reads contents from file
  void ReadFromFile();
  /// Deletes the file from disk if it exists
  bool DeleteFile();

  /// Checks if a save with mName already exists on disk
  bool FileExists();

  void SetBool(const std::string& key, bool value);
  void SetInt(const std::string& key, int value);
  void SetDouble(const std::string& key, double value);
  void SetString(const std::string& key, std::string value);

  bool ExistsBool(const std::string& key);
  bool ExistsInt(const std::string& key);
  bool ExistsDouble(const std::string& key);
  bool ExistsString(const std::string& key);

  bool GetBool(const std::string& name) const;
  int GetInt(const std::string& name) const;
  double GetDouble(const std::string& name) const;
  std::string GetString(const std::string& name) const;

  const std::string mName;

  static void ExposeLuaInterface(lua_State* state);

 private:
  void ProcessFile(FileData* data);

  std::unordered_map<std::string, bool> mBools;
  std::unordered_map<std::string, int> mInts;
  std::unordered_map<std::string, double> mDoubles;
  std::unordered_map<std::string, std::string> mStrings;

  static const std::string FILE_EXTENSION;
};

#endif
