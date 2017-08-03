#ifndef L_ENGINE_CMD_ARGS
#define L_ENGINE_CMD_ARGS

#include <string>
#include "Defines.h"

struct CommandLineArgs{
        CommandLineArgs();
        void ParseArgs(int argc, char *argv[]);

        std::string GetValue(L_CMD_ENUM value);
		bool Exists(L_CMD_ENUM);

    private:
        char*       switchNames [L_CMD_LAST];
        std::string switchValues[L_CMD_LAST];
};

#endif // L_ENGINE_CMD_ARGS
