#include "Defines.h"

#include <iostream>
#include <sstream>
#include <time.h>

const std::string BLANK_TILE_NAME="BLANK_TILE";
const std::string BLANK_ANIMATION_NAME="BLANK_ANIMATION";
const std::string BLANK_SPRITE_NAME="BLANK_SPRITE";
const std::string BLANK_HMAP_NAME="BLANK_HMAP";

const char* global_TiledStrings[TILED_EVT_LAST]{
    "EVT_MAP_ENTRANCE",
    "EVT_MAP_EXIT",
    "EVT_MAP_CHANGE",
    "CAMERA"
};

Color4f Color4f_WHITE(1.0f, 1.0f, 1.0f, 1.0f);
Color4f Color4f_RED  (1.0f, 0.0f, 0.0f, 1.0f);
Color4f Color4f_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
Color4f Color4f_BLUE (0.0f, 0.0f, 1.0f, 1.0f);
Color4f Color4f_BLACK(0.0f, 0.0f, 0.0f, 1.0f);

//Vsync should keep the FPS at roughly 60
const int TICKS_PER_SECOND  = 68;
const int SKIP_TICKS        = 1000 / TICKS_PER_SECOND;
const int MAX_FRAMESKIP     = 8;

const int CAMERA_W=480;
const int CAMERA_H=320;

const int COLLISION_GRID_SIZE = 64;

int SCREEN_W=960;
int SCREEN_H=640;

float ASPECT_RATIO=   (float)SCREEN_W / (float)SCREEN_H;

bool GET_ENDIAN(){
    int32_t i = 1;
    unsigned char *p = (unsigned char *)&i;

    if (p[0] == 1){
        return LENGINE_DEF_LITTLE_ENDIAN;
    }
    else{
        return LENGINE_DEF_BIG_ENDIAN;
    }
    //TCP/IP uses Big endian
    //FileFormat will use big endian
}

int32_t INT32_MAKE_BIG_ENDIAN(int32_t i){
    if(GET_ENDIAN()==LENGINE_DEF_LITTLE_ENDIAN){
        return INT32_REVERSE_BYTES(i);
    }
    else{return i;}
}

int32_t INT32_MAKE_LITTLE_ENDIAN(int32_t i){
    if(GET_ENDIAN()==LENGINE_DEF_BIG_ENDIAN){
        return INT32_REVERSE_BYTES(i);
    }
    else{return i;}
}

int32_t INT32_REVERSE_BYTES(int32_t i){
    int32_t returnint;
    char* num1=(char*) &i;
    char* num2=(char*) &returnint;

    num2[0]=num1[3];
    num2[1]=num1[2];
    num2[2]=num1[1];
    num2[3]=num1[0];

    return returnint;
}

uint64_t INT64_MAKE_BIG_ENDIAN(uint64_t i){
    if(GET_ENDIAN()==LENGINE_DEF_LITTLE_ENDIAN){
        return INT64_REVERSE_BYTES(i);
    }
    else{return i;}
}

uint64_t INT64_MAKE_LITTLE_ENDIAN(uint64_t i){
    if(GET_ENDIAN()==LENGINE_DEF_BIG_ENDIAN){
        return INT64_REVERSE_BYTES(i);
    }
    else{return i;}
}

uint64_t INT64_REVERSE_BYTES(uint64_t i){
    uint64_t returndob;
    char* num1=(char*) &i;
    char* num2=(char*) &returndob;

    num2[0]=num1[7];
    num2[1]=num1[6];
    num2[2]=num1[5];
    num2[3]=num1[4];
    num2[4]=num1[3];
    num2[5]=num1[2];
    num2[6]=num1[1];
    num2[7]=num1[0];

    return returndob;
}


//Double packing and unpacking functions, not mine
uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

std::string GetDate(){
	time_t rawTime;
	time(&rawTime);		
	tm* timeInfo = localtime(&rawTime);

	int bufSize = 32;
	char timeStringBuffer[bufSize];
	//Format 08-16-2017
	strftime(timeStringBuffer, bufSize, "%m-%d-%Y", timeInfo);

	return std::string(timeStringBuffer);
}

std::string GetTime(){
	time_t rawTime;
	time(&rawTime);		
	tm* timeInfo = localtime(&rawTime);

	int bufSize = 32;
	char timeStringBuffer[bufSize];
	//Format 11:30PM
	strftime(timeStringBuffer, bufSize, "%I:%M%p", timeInfo);

	return std::string(timeStringBuffer);
}

std::string GetDateTime(){
	time_t rawTime;
	time(&rawTime);		
	tm* timeInfo = localtime(&rawTime);

	int bufSize = 100;
	char timeStringBuffer[bufSize];
	//Format 08-16-2017 11:30PM
	strftime(timeStringBuffer, bufSize, "%m-%d-%Y_%I:%M%p", timeInfo);

	return std::string(timeStringBuffer);
}
#ifdef DEBUG_MODE
	void _ASSERT(char const* file, unsigned int line, char const* assertion, bool stop){
		std::cout << std::endl << "Assertion failed: " << file << ", line " << line << std::endl 
		   << "Assertion: '" << assertion << "'" << std::endl;

		if(stop){
			abort();	
		}
	}

	void* operator new(size_t size) {
		void* ptr = malloc(size);
		if(!ptr){
			throw std::bad_alloc();
		}
		return ptr;
	}
	void* operator new[](size_t size){
		void* ptr = malloc(size);
		if(!ptr){
			throw std::bad_alloc();
		}
		return ptr;
	}
	void operator delete(void* p)throw(){
		free(p);
	}
	void operator delete[](void* p)throw(){
		free(p);
	}
#endif
