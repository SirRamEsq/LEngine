#include "CommandLineArgs.h"
#include "Errorlog.h"
#include <string.h>

CommandLineArgs::CommandLineArgs() {
  switchNames[L_CMD_LEVELNAME] = "-map";
  switchNames[L_CMD_RESOLUTION_W] = "-rW";
  switchNames[L_CMD_RESOLUTION_H] = "-rH";
  switchNames[L_CMD_DEBUG] = "-debug";
}

void CommandLineArgs::ParseArgs(int argc, char *argv[]) {
  for (int ii = L_CMD_FIRST; ii < L_CMD_LAST; ii++) {
    switchValues[ii] = "";
  }

  // start with i=1 because argv[0] is the program name
  for (int i = 1; i < argc; i += 2) {
    for (int ii = L_CMD_FIRST; ii < L_CMD_LAST; ii++) {
      if (strcmp(argv[i], switchNames[ii]) == 0) {
        switchValues[ii] = argv[i + 1];
      }
    }
  }
}

std::string CommandLineArgs::GetValue(L_CMD_ENUM value) {
  return switchValues[value];
}

bool CommandLineArgs::Exists(L_CMD_ENUM value) {
  return (switchValues[value] == "");
}
