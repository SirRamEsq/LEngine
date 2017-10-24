#include "../../Engine/Errorlog.h"
#include "../../Engine/physfs.h"
#include "../catch.hpp"

TEST_CASE("Log can write to file", "[log]") {
  PHYSFS_init(NULL);
  std::string searchPath = "Data/";
  PHYSFS_addToSearchPath(searchPath.c_str(), 0);
  PHYSFS_setWriteDir("Data/");
  Log testLog;
  // Using C11 Lambda
  REQUIRE_NOTHROW([&]() {
    testLog.Write("TEST");
    testLog.Write("Error1", Log::SEVERITY::FATAL);
    testLog.Write("Error2", Log::SEVERITY::ERROR);
    testLog.Write("Error3", Log::SEVERITY::WARN);
    testLog.Write("Error4", Log::SEVERITY::INFO);
    testLog.Write("Error5", Log::SEVERITY::DEBUG);
    testLog.Write("Error6", Log::SEVERITY::TRACE);

    testLog.WriteEntriesToFile(testLog.GetEntries(), "UnitTestLog1");
  }());
}

TEST_CASE("Log can write to file ", "[log]") {
  PHYSFS_init(NULL);
  std::string searchPath = "Data/";
  PHYSFS_addToSearchPath(searchPath.c_str(), 0);
  PHYSFS_setWriteDir("Data/");
  Log testLog;
  testLog.WriteToFile("UnitTestLog2");
  // Using C11 Lambda
  REQUIRE_NOTHROW([&]() {
    testLog.Write("TEST");
    testLog.Write("Error1", Log::SEVERITY::FATAL);
    testLog.Write("Error2", Log::SEVERITY::ERROR);
    testLog.Write("Error3", Log::SEVERITY::WARN);
    testLog.Write("Error4", Log::SEVERITY::INFO);
    testLog.Write("Error5", Log::SEVERITY::DEBUG);
    testLog.Write("Error6", Log::SEVERITY::TRACE);
  }());
}
