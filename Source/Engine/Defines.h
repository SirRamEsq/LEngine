#ifndef L_DEFINES
#define L_DEFINES

#define MASK_R 255
#define MASK_G 0
#define MASK_B 255

//deifne glew static linkage if it isn't already defined
#ifndef GLEW_STATIC
	#define GLEW_STATIC
#endif

#include <string>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>


#include <memory>
#include <sstream>

#define LENGINE_DEF_TILE_W 16
#define LENGINE_DEF_TILE_H 16

#define LENGINE_DEF_LITTLE_ENDIAN 0
#define LENGINE_DEF_BIG_ENDIAN    1
#define LENGINE_DEF_SCREEN_BPP    32

#define LENGINE_MAX_ENTITIES      32768 //2^15
//#define SCREEN_W 1024
//#define SCREEN_H 800

#define LENGINE_DEF_PI 3.14159265

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

enum TILE_FLAGS{
    TF_solid=1,
    TF_useHMap=2,
    TF_destructable=4,
    TF_instantDeath=8,
    TF_swim=16
};
enum BG_FLAGS{
    BG_REPEATX=1,
    BG_REPEATY=2
};

enum TiledEventTypes{
    TILED_EVT_FIRST         = 0,
    TILED_EVT_MAP_ENTRANCE  = 0, //an area that can be used as an entrance from another map and that can used to pop the current map
    TILED_EVT_MAP_EXIT      = 1, //An area that can be used to enter a new map
    TILED_EVT_MAP_CHANGE    = 2,
    TILED_CAMERA            = 3,
    TILED_EVT_LAST          = 4
};

enum TiledObjectFlagsCamera{
    TILED_OBJECT_IS_MAIN_CAMERA = 2
};


//for Command Line
enum L_CMD_ENUM {
    L_CMD_FIRST=0,
    L_CMD_LEVELNAME=0,
    L_CMD_RESOLUTION_W=1,
    L_CMD_RESOLUTION_H=2,
    L_CMD_LAST=3
};

//Origin of a drawn image
enum LOrigin {
    L_ORIGIN_CENTER,
    L_ORIGIN_TOP_LEFT,
    L_ORIGIN_BOTTOM_LEFT,
    L_ORIGIN_TOP_RIGHT,
    L_ORIGIN_BOTTOM_RIGHT
};
typedef int MAP_DEPTH;

//Event Handling Messages
enum MESSAGE_TYPE{
    MSG_DAMAGE=0,
    MSG_MOVE,

    MSG_ENT_COLLISION,
    MSG_TILE_COLLISION,

    MSG_KEYDOWN,
    MSG_KEYUP,

    MSG_LISTEN_TO_KEY,

    MSG_MAP_PUSH,
    MSG_MAP_POP,
    MSG_MAP_CHANGE,
    MSG_LUA_EVENT,
	MSG_ENTITY_DELETED
};

typedef unsigned long int EID; //EID stands for "Entity ID"

#define EID_SYSTEM      0 //EID that refers to the system
#define EID_STATEMAN    1 //EID that refers to the statemanager
#define EID_MAPSCRIPT   2 //EID for the script belonging to the whole map
#define EID_ALLOBJS     3 //EID that broadcasts to all components
#define EID_MIN         10 //First EID that can be used by components

//Endian related functions
bool GET_ENDIAN();
int32_t INT32_MAKE_BIG_ENDIAN(int32_t i);
int32_t INT32_MAKE_LITTLE_ENDIAN(int32_t i);
int32_t INT32_REVERSE_BYTES(int32_t i);

uint64_t INT64_MAKE_BIG_ENDIAN(uint64_t i);
uint64_t INT64_MAKE_LITTLE_ENDIAN(uint64_t i);
uint64_t INT64_REVERSE_BYTES(uint64_t i);

class L_COLOR{
    public:
        L_COLOR(){
            mR=1.0f;
            mG=1.0f;
            mB=1.0f;
            mA=1.0f;
        }
        L_COLOR(double r, double g, double b, double a){
            mR=r;
            mG=g;
            mB=b;
            mA=a;
        }
        double mR;
        double mG;
        double mB;
        double mA;

        inline L_COLOR& operator =(L_COLOR& color){
            mR=color.mR;
            mG=color.mG;
            mB=color.mB;
            mA=color.mA;

            return color;
        }
};

extern L_COLOR L_COLOR_WHITE;
extern L_COLOR L_COLOR_RED  ;
extern L_COLOR L_COLOR_GREEN;
extern L_COLOR L_COLOR_BLUE ;
extern L_COLOR L_COLOR_BLACK;

//coordinate to tile grid and vise versa
inline void CoordToGrid(int &x, int &y){
    x=x/LENGINE_DEF_TILE_W;
    y=y/LENGINE_DEF_TILE_H;
}

inline void GridToCoord(int &x, int &y){
    x=x*LENGINE_DEF_TILE_W;
    y=y*LENGINE_DEF_TILE_H;
}

class Coord2d{
    public:
        Coord2d()               {x=0; y=0;  }
        Coord2d(int xx, int yy) {x=xx; y=yy;}
        int x, y;

        inline Coord2d operator+ (const Coord2d& c){
            Coord2d newCoord;
            newCoord.x = x + c.x;
            newCoord.y = y + c.y;
            return newCoord;
        }
};
class Coord2df{
    public:
        Coord2df()                     {x=0; y=0;  }
        Coord2df(double xx, double yy) {x=xx; y=yy;}
        double x, y;

        inline Coord2df operator+ (const Coord2df& c) const{
            Coord2df newCoord;
            newCoord.x = x + c.x;
            newCoord.y = y + c.y;
            return newCoord;
        }
        inline Coord2df operator- (const Coord2df& c) const{
            Coord2df newCoord;
            newCoord.x = x - c.x;
            newCoord.y = y - c.y;
            return newCoord;
        }
};

class CRect{
    public:
        CRect(const int& xx, const int& yy, const int& wi, const int& he){SetValues(xx, yy, wi, he);}
        CRect(){SetValues(0,0,0,0);}
        void SetValues(const int& xx, const int& yy,const int& wi, const int& he){
            x=xx; y=yy; w=wi; h=he;
        }

        inline int  GetWidth()              const   {return w;  }
        inline void SetWidth(const int& ww)         {w=ww;      }

        inline int  GetHeight()             const   {return h;  }
        inline void SetHeight(const int& hh)        {h=hh;      }

        inline int  GetX()                  const   {return x;  }
        inline void SetX(const int& xx)             {x=xx;      }

        inline int  GetY()                  const   {return y;  }
        inline void SetY(const int& yy)             {y=yy;      }

        int GetTop     ()  const;
        int GetBottom  ()  const;
        int GetLeft    ()  const;
        int GetRight   ()  const;

        int x, y, w, h;
};

class FloatRect{
    public:
        FloatRect(const float& xx, const float& yy, const float& wi, const float& he){SetValues(xx, yy, wi, he);}
        FloatRect(){SetValues(0,0,0,0);}
        void SetValues(const float& xx, const float& yy, const float& wi, const float& he){
            x=xx; y=yy; w=wi; h=he;
        }

        inline float GetWidth()       const     {return w;  }
        inline void  SetWidth(const float& ww)  {w=ww;      }

        inline float GetHeight()      const     {return h;  }
        inline void  SetHeight(const float& hh) {h=hh;      }

        inline float GetX()           const     {return x;  }
        inline void  SetX(const float& xx)      {x=xx;      }

        inline float GetY()           const     {return y;  }
        inline void  SetY(const float& yy)      {y=yy;      }

        inline float GetTop     ()  const;
        inline float GetBottom  ()  const;
        inline float GetLeft    ()  const;
        inline float GetRight   ()  const;

        float x, y, w, h;
};

//These data structures can be treated as arrays by openGL
struct LVertexPos2D  { GLfloat x;             GLfloat y;          };
struct LTexCoord     { GLfloat s;             GLfloat t;          };
struct LVertexData2D { LVertexPos2D position; LTexCoord texCoord; };

inline bool CollisionRect(const CRect& RR, const CRect& R){
    if (R.GetBottom() < RR.GetTop())   {return 0;}
    if (R.GetTop()    > RR.GetBottom()){return 0;}
    if (R.GetRight()  < RR.GetLeft())  {return 0;}
    if (R.GetLeft()   > RR.GetRight()) {return 0;}
    return 1;
}

inline bool CollisionPoint(int xx, int yy, const CRect& R){
    int leftR, rightR, topR, bottomR;

    leftR= R.GetLeft();
    topR= R.GetTop();
    rightR= R.GetRight();
    bottomR= R.GetBottom();

    if(((xx<=rightR)&&(xx>=leftR))){ //check for a horizontal collision
        if(((yy>=topR)&&(yy<=bottomR))){//check for a vertical collision
            return 1;
        }
    }
    return 0;
}

extern const int COLLISION_GRID_SIZE;
extern const std::string BLANK_TILE_NAME;
extern const std::string BLANK_ANIMATION_NAME;
extern const std::string BLANK_SPRITE_NAME;
extern const std::string BLANK_HMAP_NAME;

extern const char* global_TiledStrings[TILED_EVT_LAST];

extern const int TICKS_PER_SECOND;
extern const int SKIP_TICKS;
extern const int MAX_FRAMESKIP;

extern int SCREEN_W;
extern int SCREEN_H;

extern const int CAMERA_W;
extern const int CAMERA_H;

extern float ASPECT_RATIO;

template <typename T>
std::string NumberToString ( T Number ){
	std::stringstream ss;
	ss << Number;
	return ss.str();
}

template <typename T>
T StringToNumber ( const std::string &Text ){//Text not by const reference so that the function can be used with a                              //character array as argument
	std::stringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}

//Herb sutters make_unique function
//https://herbsutter.com/gotw/_102/
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args ){
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

//Double packing, not mine
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))
uint64_t pack754(long double f, unsigned bits, unsigned expbits);
long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

#endif
