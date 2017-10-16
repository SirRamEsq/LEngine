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

#include "math.h"
#include <memory>
#include <sstream>

#define LENGINE_DEF_TILE_W 16
#define LENGINE_DEF_TILE_H 16

#define LENGINE_DEF_LITTLE_ENDIAN 0
#define LENGINE_DEF_BIG_ENDIAN    1
#define LENGINE_DEF_SCREEN_BPP    32

#define LENGINE_MAX_ENTITIES      32768 //2^15

#define EID_SYSTEM      0 //EID that refers to the system
#define EID_STATEMAN    1 //EID that refers to the statemanager
#define EID_MAPSCRIPT   2 //EID for the script belonging to the whole map
#define EID_ALLOBJS     3 //EID that broadcasts to all components

//EID that is reserved for an entity that is initialized along with a GameState
#define EID_RESERVED_STATE_ENTITY 5

#define EID_MIN         10 //First EID that can be used by components
//#define SCREEN_W 1024
//#define SCREEN_H 800

#define LENGINE_DEF_PI 3.14159265

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

enum TILE_FLAGS{
    TF_solid=1,
    TF_useHMap=2,
    TF_destructable=4,
    TF_instantDeath=8
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
	L_CMD_DEBUG=3,
    L_CMD_LAST=4
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
typedef unsigned long int EID; //EID stands for "Entity ID"

template <typename T>
T StringToNumber (const std::string& Text ){
	std::stringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}

//Endian related functions
bool GET_ENDIAN();
int32_t INT32_MAKE_BIG_ENDIAN(int32_t i);
int32_t INT32_MAKE_LITTLE_ENDIAN(int32_t i);
int32_t INT32_REVERSE_BYTES(int32_t i);

uint64_t INT64_MAKE_BIG_ENDIAN(uint64_t i);
uint64_t INT64_MAKE_LITTLE_ENDIAN(uint64_t i);
uint64_t INT64_REVERSE_BYTES(uint64_t i);

//Time functions
std::string GetDate();
std::string GetTime();
std::string GetDateTime();

class Color4f{
    public:
        Color4f(){
            r=1.0f;
            g=1.0f;
            b=1.0f;
            a=1.0f;
        }
        Color4f(float rr, float gg, float bb, float aa){
            r = rr;
            g = gg;
			b = bb;
			a = aa;
        }
        float r;
        float g;
        float b;
        float a;

        inline Color4f& operator =(Color4f& color){
            r=color.r;
            g=color.g;
            b=color.b;
            a=color.a;

            return color;
        }
};

//These data structures can be treated as arrays by openGL
struct LVertexPos2D  { GLfloat x;             GLfloat y;          };
struct LTexCoord     { GLfloat s;             GLfloat t;          };
struct LVertexData2D { LVertexPos2D position; LTexCoord texCoord; };

extern Color4f Color4f_WHITE;
extern Color4f Color4f_RED  ;
extern Color4f Color4f_GREEN;
extern Color4f Color4f_BLUE ;
extern Color4f Color4f_BLACK;

extern const int COLLISION_GRID_SIZE;
extern const std::string BLANK_TILE_NAME;
extern const std::string BLANK_ANIMATION_NAME;
extern const std::string BLANK_SPRITE_NAME;
extern const std::string BLANK_HMAP_NAME;

extern const char* global_TiledStrings[TILED_EVT_LAST];

extern const int TICKS_PER_SECOND;
extern const int SKIP_TICKS;
extern const int MAX_FRAMESKIP;

template <typename T>
std::string NumberToString ( T Number ){
	std::stringstream ss;
	ss << Number;
	return ss.str();
}

//Herb sutters std::make_unique function
//https://herbsutter.com/gotw/_102/
/*
template<typename T, typename ...Args>
std::unique_ptr<T> std::make_unique( Args&& ...args ){
    return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}
*/

//Double packing, not mine
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))
uint64_t pack754(long double f, unsigned bits, unsigned expbits);
long double unpack754(uint64_t i, unsigned bits, unsigned expbits);


//custom assert function 'ASSERT'
#ifdef DEBUG_MODE
	void _ASSERT(char const* file, unsigned int line, char const* assertion, bool stop);

	#define ASSERT(f)	\
		if (f)			\
			{}			\
		else 			\
			_ASSERT(__FILE__, __LINE__, #f, true)

	#define ASSERT_CONTINUE(f)	\
		if (f)			\
			{}			\
		else 			\
			_ASSERT(__FILE__, __LINE__, #f, false)

	void* operator new(size_t size) throw(std::bad_alloc);
	void* operator new[](size_t size) throw(std::bad_alloc);
	void  operator delete(void* p) throw();
	void  operator delete[](void* p) throw();

#else
	#define ASSERT(f)
	#define ASSERT_CONTINUE(f)	

#endif

#endif

