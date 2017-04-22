#ifndef RL_LEVENT
#define RL_LEVENT

#include <stdint.h>
#include <string>

class LEvent{
    public:
        LEvent(std::string script, int32_t flgs=0);
        std::string scriptName;
        int32_t flags;
};

#endif
