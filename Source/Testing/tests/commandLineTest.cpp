#include "../catch.hpp"
#include "../../Engine/CommandLineArgs.h"

TEST_CASE("Test CommandLineArgs", "[commandLine]") {
  CommandLineArgs cmd;
  int argc = 4;
  char *arg0 = "programName";
  char *arg1 = "-f";
  char *arg2 = "-luaTestDir";
  char *arg3 = "-/Data/Resources/Scripts/Testing";

  char *argv[argc];
  argv[0] = arg0;
  argv[1] = arg1;
  argv[2] = arg2;
  argv[3] = arg3;

  cmd.ParseArgs(argc, argv);

  bool checkExists = false;

  checkExists = cmd.Exists(std::string("-f"));
  REQUIRE(checkExists == true);

  checkExists = cmd.Exists(std::string("-luaTestDir"));
  REQUIRE(checkExists == true);

  checkExists = cmd.Exists(std::string("Scripts"));
  REQUIRE(checkExists == false);

  std::string value;

  value = cmd.GetValue(std::string("-luaTestDir"));
  REQUIRE(*value.c_str() == *arg3);

  value = cmd.GetValue(std::string("-f"));
  REQUIRE(*value.c_str() == *arg2);
}
