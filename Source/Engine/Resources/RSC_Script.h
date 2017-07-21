#ifndef L_SCRIPT
#define L_SCRIPT

#include <string>
#include <memory>

//find a way to compile the lua string into bytecode and store it here,
//that way the lua interface won't have to compile every time it wants to run an RSC_Script
class RSC_Script{
    public:
        RSC_Script(std::string sname, const char* dat, unsigned int fsize);
        ~RSC_Script();

        const std::string script;
        const std::string scriptName;
        static std::unique_ptr<RSC_Script> LoadResource(const std::string& fname);
};

#endif
