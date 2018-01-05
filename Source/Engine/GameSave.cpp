#include "GameSave.h"
#include "physfs.h"

const std::string GameSave::FILE_EXTENSION = ".sav";

/////////////////////
// HelperFunctions //
/////////////////////
void SaveDirectoryExists() {
  std::string saveDir = "Saves/";
  auto result = PHYSFS_isDirectory(saveDir.c_str());
  if (result == 0) {
    auto errorCode = PHYSFS_mkdir(saveDir.c_str());
    if (errorCode == 0) {
      throw LEngineException("Cannot create Saves Dir");
    }
  }
}
bool ReadBool(const char* bufferStart, unsigned int& readHead) {
  char buffer;

  buffer = bufferStart[readHead + readHead];
  // increment readHead
  readHead += 8;

  uint64_t* intPointer = (uint64_t*)&buffer;
  bool returnValue = true;
  if (*intPointer == 0) {
    returnValue = false;
  }

  return returnValue;
}

int ReadInt(const char* bufferStart, unsigned int& readHead) {
  char buffer[8];
  for (auto i = 0; i < 8; i++) {
    buffer[i] = bufferStart[readHead + i];
  }
  // increment readHead
  readHead += 8;

  int64_t* intPointer = (int64_t*)&buffer[0];
  if (GET_ENDIAN() == LENGINE_DEF_LITTLE_ENDIAN) {
    *intPointer = INT64_REVERSE_BYTES(*intPointer);
  }

  return *intPointer;
}

double ReadDouble(const char* bufferStart, unsigned int& readHead) {
  char buffer[8];
  for (auto i = 0; i < 8; i++) {
    buffer[i] = bufferStart[readHead + i];
  }
  // increment readHead
  readHead += 8;

  uint64_t* intPointer = (uint64_t*)&buffer[0];
  if (GET_ENDIAN() == LENGINE_DEF_LITTLE_ENDIAN) {
    *intPointer = INT64_REVERSE_BYTES(*intPointer);
  }
  double unpackedDouble = unpack754_64(*intPointer);

  return unpackedDouble;
}

std::string ReadString(const char* bufferStart, unsigned int& readHead) {
  // String will read all bytes till it hits a '\0'
  std::string newString(&bufferStart[readHead]);
  // increment readHead
  readHead += newString.size();
  // plus null terminator
  readHead += 1;

  return newString;
}
template <class T>
unsigned int GetSize(T value) {
  // Default
  return sizeof(value);
}
template <>
unsigned int GetSize(std::string value) {
  // string length
  return value.length();
}

template <class T>
void WriteKeyValuePairToFile(const std::string& fileName, PHYSFS_File* file,
                             const std::string& key, const T* value,
                             unsigned int valueLength) {
  auto keyLength = GetSize(key);
  auto bytesWrittenKey = PHYSFS_write(file, key.c_str(), keyLength, 1);
  // write null terminator
  PHYSFS_write(file, "\0", 1, 1);
  auto bytesWrittenValue = PHYSFS_write(file, value, valueLength, 1);

  if (bytesWrittenKey < 1) {
    LOG_ERROR("Less than one object written for the Key");
  }
  if (bytesWrittenValue < 1) {
    LOG_ERROR("Less than one object written for the Value");
  }

  const char* physfsError = PHYSFS_getLastError();
  if (physfsError != NULL) {
    std::stringstream ss;
    ss << "Physfs Error While saving key'" << key << "' to file '" << fileName
       << "'" << std::endl
       << "  Error: " << physfsError;
    LOG_ERROR(ss.str());
    throw LEngineFileException(ss.str(), fileName);
  }
}

bool CheckErrorPHYSFS(const std::string& fileName) {
  const char* physfsError = PHYSFS_getLastError();
  if (physfsError != NULL) {
    std::stringstream ss;
    ss << "Physfs Error " << physfsError << std::endl;
    LOG_ERROR(ss.str());
    return false;
  }
  return true;
}

//////////////
// GameSave //
//////////////
GameSave::GameSave(const std::string& name) : mName(name) {
  // Clear out error
  PHYSFS_getLastError();

  // Throws if save directory cannot be created
  SaveDirectoryExists();

  // Clear out error
  PHYSFS_getLastError();
}

void GameSave::WriteToFile() {
  // Clear out error
  PHYSFS_getLastError();

  std::string fullPath = "Saves/" + mName + FILE_EXTENSION;
  auto file = PHYSFS_openWrite(fullPath.c_str());
  CheckErrorPHYSFS(mName);
  try {
    uint64_t bools = mBools.size();
    bools = INT64_MAKE_BIG_ENDIAN(bools);
    auto objectsWrittenValue = PHYSFS_write(file, &bools, sizeof(uint64_t), 1);
    CheckErrorPHYSFS(mName);
    for (auto i = mBools.begin(); i != mBools.end(); i++) {
      // bool size is implementation defined
      std::string key = i->first;
      bool truth = i->second;

      // Store either all ones or all zeros based on truth
      uint64_t valueToStore = 0;
      if (truth) {
        valueToStore = 0xFFFFFFFFFFFFFFFF;
      }

      WriteKeyValuePairToFile(mName, file, key, &valueToStore,
                              GetSize(valueToStore));
    }

    uint64_t ints = mInts.size();
    ints = INT64_MAKE_BIG_ENDIAN(ints);
    objectsWrittenValue = PHYSFS_write(file, &ints, sizeof(uint64_t), 1);
    CheckErrorPHYSFS(mName);
    for (auto i = mInts.begin(); i != mInts.end(); i++) {
      std::string key = i->first;
      int64_t localValue = i->second;
      int64_t bigEndianValue = INT64_MAKE_BIG_ENDIAN(localValue);

      WriteKeyValuePairToFile(mName, file, key, &bigEndianValue,
                              GetSize(bigEndianValue));
    }

    uint64_t doubles = mDoubles.size();
    doubles = INT64_MAKE_BIG_ENDIAN(doubles);
    objectsWrittenValue = PHYSFS_write(file, &doubles, sizeof(uint64_t), 1);
    CheckErrorPHYSFS(mName);
    for (auto i = mDoubles.begin(); i != mDoubles.end(); i++) {
      std::string key = i->first;
      double localValue = i->second;
      int64_t packedDouble = pack754_64(localValue);
      int64_t bigEndianValue = INT64_MAKE_BIG_ENDIAN(packedDouble);

      WriteKeyValuePairToFile(mName, file, key, &bigEndianValue,
                              GetSize(bigEndianValue));
    }

    uint64_t strings = mStrings.size();
    strings = INT64_MAKE_BIG_ENDIAN(strings);
    objectsWrittenValue = PHYSFS_write(file, &strings, sizeof(uint64_t), 1);
    CheckErrorPHYSFS(mName);
    for (auto i = mStrings.begin(); i != mStrings.end(); i++) {
      std::string key = i->first;
      std::string value = i->second;

      WriteKeyValuePairToFile(mName, file, key, value.c_str(), GetSize(value));
      // write null terminator
      PHYSFS_write(file, "\0", 1, 1);
    }
  } catch (LEngineFileException e) {
    std::stringstream ss;
    ss << "Could not Save file named '" << mName << "'" << std::endl
       << e.what();
    LOG_FATAL(ss.str());
  }

  PHYSFS_close(file);
}

void GameSave::ReadFromFile() {
  try {
    std::string fullPath = "Saves/" + mName + FILE_EXTENSION;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      throw LEngineFileException("Couldn't load save named ", mName);
    }

    ProcessFile(data.get());

  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
    throw e;
  }
}

/// Checks if a save with mName already exists on disk
bool GameSave::FileExists() {
  bool exists = false;
  PHYSFS_getLastError();
  try {
    std::string fullPath = "Saves/" + mName + FILE_EXTENSION;
    exists = GenericFileExists(fullPath);

  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
  }
  PHYSFS_getLastError();
  return exists;
}

void GameSave::SetBool(const std::string& key, bool value) {
  mBools[key] = value;
}
void GameSave::SetInt(const std::string& key, int value) { mInts[key] = value; }
void GameSave::SetDouble(const std::string& key, double value) {
  mDoubles[key] = value;
}
void GameSave::SetString(const std::string& key, std::string value) {
  mStrings[key] = value;
}

bool GameSave::GetBool(const std::string& name) const {
  auto i = mBools.find(name);
  if (i == mBools.end()) {
    return false;
  }
  return i->second;
}
int GameSave::GetInt(const std::string& name) const {
  auto i = mInts.find(name);
  if (i == mInts.end()) {
    return 0;
  }
  return i->second;
}
double GameSave::GetDouble(const std::string& name) const {
  auto i = mDoubles.find(name);
  if (i == mDoubles.end()) {
    return 0;
  }
  return i->second;
}
std::string GameSave::GetString(const std::string& name) const {
  auto i = mStrings.find(name);
  if (i == mStrings.end()) {
    return 0;
  }
  return i->second;
}

bool GameSave::ExistsBool(const std::string& key) {
  auto i = mBools.find(key);
  if (i == mBools.end()) {
    return false;
  }
  return true;
}
bool GameSave::ExistsInt(const std::string& key) {
  auto i = mInts.find(key);
  if (i == mInts.end()) {
    return false;
  }
  return true;
}
bool GameSave::ExistsDouble(const std::string& key) {
  auto i = mDoubles.find(key);
  if (i == mDoubles.end()) {
    return false;
  }
  return true;
}
bool GameSave::ExistsString(const std::string& key) {
  auto i = mStrings.find(key);
  if (i == mStrings.end()) {
    return false;
  }
  return true;
}

void GameSave::ProcessFile(FileData* data) {
  const char* buffer = data->GetData();
  const char* bufferStart = &buffer[0];
  unsigned int readHead = 0;
  int boolCount = ReadInt(bufferStart, readHead);
  for (auto i = 0; i != boolCount; i++) {
    auto newKey = ReadString(bufferStart, readHead);
    auto newValue = ReadBool(bufferStart, readHead);
    mBools[newKey] = newValue;
  }
  int intCount = ReadInt(bufferStart, readHead);
  for (auto i = 0; i != intCount; i++) {
    auto newKey = ReadString(bufferStart, readHead);
    auto newValue = ReadInt(bufferStart, readHead);
    mInts[newKey] = newValue;
  }
  int doubleCount = ReadInt(bufferStart, readHead);
  for (auto i = 0; i != doubleCount; i++) {
    auto newKey = ReadString(bufferStart, readHead);
    auto newValue = ReadDouble(bufferStart, readHead);
    mDoubles[newKey] = newValue;
  }
  int stringCount = ReadInt(bufferStart, readHead);
  for (auto i = 0; i != stringCount; i++) {
    auto newKey = ReadString(bufferStart, readHead);
    auto newValue = ReadString(bufferStart, readHead);
    mStrings[newKey] = newValue;
  }
}

bool GameSave::DeleteFile() {
  if (!FileExists()) {
    return true;
  }

  std::string fullPath = "Saves/" + mName + FILE_EXTENSION;
  auto errorCode = PHYSFS_delete(fullPath.c_str());
  if (errorCode == 0) {
    const char* physfsError = PHYSFS_getLastError();
    std::stringstream ss;
    ss << "Couldn't delete save file '" << mName << "'" << std::endl;
    if (physfsError != NULL) {
      ss << "Physfs Error " << physfsError;
      throw LEngineFileException(ss.str(), mName);
    }
    LOG_ERROR(ss.str());
    return false;
  }
  return true;
}
