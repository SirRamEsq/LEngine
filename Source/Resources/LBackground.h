#ifndef RL_LBACKGROUND
#define RL_LBACKGROUND

#include <string>

class LBackground{
    public:
        LBackground(std::string spr, std::string ani, int xx=0, int yy=0, bool xrpt=true, bool yrpt=true);
        LBackground(std::string spr, std::string ani, int xx, int yy, int f, double paralax=1.0f, double speed=1.0f);
        LBackground();

        LBackground& operator =(LBackground& bg);

        std::string spriteName, animationName;
        int x, y;
        unsigned int width, height;
        int flags;
        double paralaxSpeed;
        double animationSpeed;
};

#endif
