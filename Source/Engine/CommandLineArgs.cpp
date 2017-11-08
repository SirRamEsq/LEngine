#include "CommandLineArgs.h"
#include "Errorlog.h"
#include <string.h>
#include <algorithm>

CommandLineArgs::CommandLineArgs() {}

void CommandLineArgs::ParseArgs(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    tokens.push_back(std::string(argv[i]));
  }
}

std::string CommandLineArgs::GetValue(std::string name) {
  /*
for (auto i = tokens.begin(); i != tokens.end(); i++) {
  if ((*i == name) and (i + 1 != tokens.end())) {
    return *(i + 1);
  }
}

return "";
*/
  std::vector<std::string>::const_iterator itr;
  itr = std::find(tokens.begin(), tokens.end(), name);
  if (itr != tokens.end() && ++itr != tokens.end()) {
    return *itr;
  }
  const std::string empty_string("");
  return empty_string;
}

bool CommandLineArgs::Exists(std::string name) {
  /*
  for (auto i = tokens.begin(); i != tokens.end(); i++){
      if (*i == name){
          return true;
      }
  }
  return false;
  */
  return std::find(tokens.begin(), tokens.end(), name) != tokens.end();
}
