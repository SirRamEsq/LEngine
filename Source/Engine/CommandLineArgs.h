#ifndef L_ENGINE_CMD_ARGS
#define L_ENGINE_CMD_ARGS

#include <string>
#include <vector>

class CommandLineArgs {
 public:
  CommandLineArgs();
  void ParseArgs(int argc, char *argv[]);

  std::string GetValue(std::string name);
  bool Exists(std::string name);

 private:
  std::vector<std::string> tokens;
};

#endif  // L_ENGINE_CMD_ARGS
