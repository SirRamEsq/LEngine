#include "utility.h"

void InitTestPhysfs(){
  PHYSFS_init(NULL);
  std::string searchPath = "Data/";
  PHYSFS_addToSearchPath(searchPath.c_str(), 0);
  PHYSFS_setWriteDir("Data/");
  const char *physfsError = PHYSFS_getLastError();
  REQUIRE(physfsError == NULL);
}

void CloseTestPhysfs(){
  PHYSFS_deinit();
}

