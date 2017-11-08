// This tells Catch to provide a main() - only do this in one cpp file
//#define CATCH_CONFIG_MAIN

// in one cpp file
#define CATCH_CONFIG_RUNNER
#include "main.h"
#include "catch.hpp"

#include "../Engine/Errorlog.h"
#include "../Engine/CommandLineArgs.h"

struct TEST_DIR_BLANK : std::exception {
  const char* what() const noexcept { return "LUA_TEST_DIR is empty \n"; }
};

std::string LUA_TEST_DIR = "";

int main(int argc, char* argv[]) {
	Log::staticLog.CloseFileHandle();
  Catch::Session session;

  int result = 20;
  try {
    CommandLineArgs cmd;
    std::string luaTestArg = "--luaTestDir";

    cmd.ParseArgs(argc, argv);
    LUA_TEST_DIR = cmd.GetValue(luaTestArg);

    if (LUA_TEST_DIR == "") {
      throw new TEST_DIR_BLANK();
    }

    int returnCode = session.applyCommandLine(
        argc, argv, Catch::Session::OnUnusedOptions::Ignore);
    if (returnCode != 0) return returnCode;

    result = session.run();
  } catch (std::exception e) {
    std::cout << e.what();
  }

  return (result > 0xff ? result : 0xff);
}
