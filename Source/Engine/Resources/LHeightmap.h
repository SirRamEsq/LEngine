#ifndef RL_LHMAP
#define RL_LHMAP

#include <inttypes.h>
#include <algorithm>

class LHeightmap{
    public:
        LHeightmap();
        LHeightmap(int8_t hmHparam[16], int8_t hmVparam[16]);
        LHeightmap(int8_t hmHparam[16], int8_t hmVparam[16], int angleHparam, int angleVparam);

        void SetHeightmapH (int8_t hHMap [16]);
        void SetHeightmapV (int8_t vHMap [16]);

        void SetAngleH (int a);
        void SetAngleV (int a);

        int DetermineHAngle();
        int DetermineVAngle();

        //Lua helper functions
        int GetHeightMapH (int index);
        int GetHeightMapV (int index);

        //Horizontal and veritcal heightmaps
        int8_t heightMapH [16], heightMapV [16];
        int angleH, angleV;
};

#endif
