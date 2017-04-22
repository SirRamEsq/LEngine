#ifndef L_SCRIPT
#define L_SCRIPT

#include <string>

//find a way to compile the lua string into bytecode and store it here,
//that way the lua interface won't have to compile every time it wants to run an LScript
class LScript{
    public:
        LScript(std::string sname, const char* dat, unsigned int fsize);
        ~LScript();

        const std::string script;
        const std::string scriptName;
};

#endif
