#ifndef L_OBJ
#define L_OBJ

#include <string>

class LObj{
    public:
        LObj(std::string oname, std::string sprname, std::string aniname, std::string scrname);
        ~LObj();

        std::string objName;
        std::string spriteName;
        std::string animationName;
        std::string scriptName;
};

#endif
