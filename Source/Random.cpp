#include "Random.h"
#include "SDL2/SDL.h"

RandomGenerator::RandomGenerator(){
    mSeed=0;
}

RandomGenerator::~RandomGenerator(){

}

int RandomGenerator::GenerateSeed(){
    return ( SDL_GetTicks() % sizeof(int) );
}

void RandomGenerator::SetSeed(int seed){
    mSeed=seed;
    srand(mSeed);
}

//Max range may be 32767
int RandomGenerator::GenerateRandomIntValue(int imin, int imax){
    if(imin==imax){return imin;}
    if(imin>imax){
        int temp=imin;
        imin=imax;
        imax=temp;
    }
    int difference= imax - imin;
    int randomValue=rand() % difference; //Value returned is between 0 and difference - 1

    return imin + randomValue; //Add the randomValue to the minimum value to get it in range
}

double RandomGenerator::GenerateRandomFloatValue(double imin, double imax){
    if(imin==imax){return imin;}
    if(imin>imax){
        double temp=imin;
        imin=imax;
        imax=temp;
    }
    double difference= imax - imin;
    double randomValue=static_cast <double> (rand()) / (static_cast <double> (RAND_MAX/difference)); //Value returned is between 0 and difference - 1

    return imin + randomValue; //Add the randomValue to the minimum value to get it in range
}
