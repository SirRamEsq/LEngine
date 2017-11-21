#ifndef LENGINE_TEST_UTILS
#define LENGINE_TEST_UTILS

#include "catch.hpp"

#include "../physfs.h"
#include <string>

void InitPhysfs(){
  PHYSFS_init(NULL);
  std::string searchPath = "Data/";
  PHYSFS_addToSearchPath(searchPath.c_str(), 0);
  PHYSFS_setWriteDir("Data/");
  const char *physfsError = PHYSFS_getLastError();
  REQUIRE(physfsError == NULL);
}

void ClosePhysfs(){
  PHYSFS_deinit();
}

#endif
