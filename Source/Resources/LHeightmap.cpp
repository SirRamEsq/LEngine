#include "LHeightmap.h"
#include "../Defines.h"
#include "math.h"
#include "../Errorlog.h"

double atan2Degree(double valy, double valx){
    return ((atan2(valy, valx)) * 180) / LENGINE_DEF_PI; //atan2(y,x); if 'y' is negative, output is negative
}

double ConvertToPositiveAngle(double angle){
    angle = fmod(angle, 360.0f);//Get value between 0 and 360
    while(angle < 0.0f) { //pretty sure this comparison is valid for doubles and floats
        angle += 360.0f;
    }
    return angle;
}

int ConvertInt(double val){
    return floor(val+0.5f);
}

void LHeightmap::SetHeightmapH (int8_t hHMap [16]){
    std::copy(hHMap, hHMap+16, heightMapH);
}
void LHeightmap::SetHeightmapV (int8_t vHMap [16]){
    std::copy(vHMap, vHMap+16, heightMapV);
}

void LHeightmap::SetAngleH (int a){angleH=a;}
void LHeightmap::SetAngleV (int a){angleV=a;}

int LHeightmap::DetermineHAngle(){
    /*int newAngle=0;
    int heightDiff=heightMapH[15]-heightMapH[0]; //Get the difference in between the last and first height values
                                                        //(where up is positive y)
    newAngle= atan2(heightDiff,15); //atan2(y,x)
    newAngle= newAngle * 180 / LENGINE_DEF_PI; //get angle in degrees
    return newAngle;*/
    double newAngle=0;
    int highest=0;
    int highestX=0;
    int lowest=100;
    int lowestX=0;
    for(int i=0; i<=15; i++){
        if(heightMapH[i]>highest){
            highest=heightMapH[i];
            highestX=i;
        }
        if(heightMapH[i]<lowest){
            lowest=heightMapH[i];
            lowestX=i;
        }
    }
    if(highestX==lowestX){return 0;}//If the highest is the same as the lowest, It's flat

    bool Dir=(highestX>lowestX);
    //Find height values closest to each other
    for(int i=0; i<=15; i++){
        if(Dir==true){
            if(heightMapH[i]==highest){
                if(i<highestX){
                    highestX=i;
                }
            }
            if(heightMapH[i]==lowest){
                if(i>lowestX){
                    lowestX=i;
                }
            }
        }
        else{
            if(heightMapH[i]==highest){
                if(i>highestX){
                    highestX=i;
                }
            }
            if(heightMapH[i]==lowest){
                if(i<lowestX){
                    lowestX=i;
                }
            }
        }
    }

    int heightDiff=highest-lowest; //Get the difference in between the last height value above zero and first height value above zero
    int xDiff;
    if(Dir==true){
        xDiff= highestX-lowestX;
    }
    else{xDiff= lowestX-highestX; heightDiff*=-1;}

    //Uses negative Angles                                            //(where up is positive y)
    //newAngle= atan2Degree(heightDiff,xDiff); //atan2(y,x); if 'y' is negative, output is negative
    //return newAngle;

    //Uses 360 Degrees
    newAngle= atan2Degree(heightDiff,xDiff); //atan2(y,x); if 'y' is negative, output is negative
    newAngle= ConvertToPositiveAngle(newAngle);
    return newAngle;
}
/*
int LHeightmap::DetermineHAngle(){
    double newAngle=0;
    int highest=0;
    int highestX=0;
    int lowest=100;
    int lowestX=0;
    for(int i=0; i<=15; i++){
        if(heightMapH[i]>highest){
            highest=heightMapH[i];
            highestX=i;
        }
        if(heightMapH[i]<lowest){
            lowest=heightMapH[i];
            lowestX=i;
        }
    }
    if(highestX==lowestX){return 0;}//If the highest is the same as the lowest, It's flat

    bool Dir=(highestX>lowestX);
    //Find height values closest to each other
    for(int i=0; i<=15; i++){
        if(Dir==true){
            if(heightMapH[i]==highest){
                if(i<highestX){
                    highestX=i;
                }
            }
            if(heightMapH[i]==lowest){
                if(i>lowestX){
                    lowestX=i;
                }
            }
        }
        else{
            if(heightMapH[i]==highest){
                if(i>highestX){
                    highestX=i;
                }
            }
            if(heightMapH[i]==lowest){
                if(i<lowestX){
                    lowestX=i;
                }
            }
        }
    }

    int heightDiff=highest-lowest; //Get the difference in between the last height value above zero and first height value above zero
    int xDiff;
    if(Dir==true){
        xDiff= highestX-lowestX;
    }
    else{xDiff= lowestX-highestX; heightDiff*=-1;}
                                                //(where up is positive y)
    newAngle= atan2(heightDiff,xDiff); //atan2(y,x); if 'y' is negative, output is negative
    newAngle= newAngle * 180 / LENGINE_DEF_PI; //get angle in degrees
    return newAngle;
}*/

int LHeightmap::DetermineVAngle(){
    /*int newAngle=0;
    int heightDiff=heightMapV[15]-heightMapV[0]; //Get the difference in between the last and first height values
                                                //(where up is positive y)
    newAngle= atan2(heightDiff,15); //atan2(y,x)
    newAngle= newAngle * 180 / LENGINE_DEF_PI; //get angle in degrees
    return newAngle;*/
    double newAngle=0;
    int highest=0;
    int highestX=0;
    int lowest=100;
    int lowestX=0;
    for(int i=0; i<=15; i++){
        if(heightMapV[i]>highest){
            highest=heightMapH[i];
            highestX=i;
        }
        if(heightMapV[i]<lowest){
            lowest=heightMapV[i];
            lowestX=i;
        }
    }
    if(highestX==lowestX){return 0;}//If the highest is the same as the lowest, It's flat

    bool Dir=(highestX>lowestX);
    //Find height values closest to each other
    for(int i=0; i<=15; i++){
        if(Dir==true){
            if(heightMapV[i]==highest){
                if(i<highestX){
                    highestX=i;
                }
            }
            if(heightMapV[i]==lowest){
                if(i>lowestX){
                    lowestX=i;
                }
            }
        }
        else{
            if(heightMapV[i]==highest){
                if(i>highestX){
                    highestX=i;
                }
            }
            if(heightMapV[i]==lowest){
                if(i<lowestX){
                    lowestX=i;
                }
            }
        }
    }

    int heightDiff=highest-lowest; //Get the difference in between the last height value above zero and first height value above zero
    int xDiff;
    if(Dir==true){
        xDiff= highestX-lowestX;
    }
    else{xDiff= lowestX-highestX; heightDiff*=-1;}
                                                //(where up is positive y)
    newAngle= atan2(heightDiff,xDiff); //atan2(y,x)
    newAngle= newAngle * 180 / LENGINE_DEF_PI; //get angle in degrees
    return newAngle;
}

LHeightmap::LHeightmap(){
    int8_t hm[16];
    int8_t vm[16];

    for(unsigned int i=0; i<=15; i++){
        hm[i]=16;
        vm[i]=16;
    }

    SetHeightmapH(hm);
    SetHeightmapV(vm);
    SetAngleH(DetermineHAngle());
    SetAngleV(DetermineVAngle());
}

LHeightmap::LHeightmap(int8_t hmHparam[16], int8_t hmVparam[16]){
    SetHeightmapH(hmHparam);
    SetHeightmapV(hmVparam);
    SetAngleH(DetermineHAngle());
    SetAngleV(DetermineVAngle());
}

LHeightmap::LHeightmap(int8_t hmHparam[16], int8_t hmVparam[16], int angleHparam, int angleVparam){
    SetHeightmapH(hmHparam);
    SetHeightmapV(hmVparam);
    SetAngleH(angleHparam);
    SetAngleV(angleVparam);
}

int LHeightmap::GetHeightMapH (int index){
    return heightMapH[index];
}

int LHeightmap::GetHeightMapV (int index){
    return heightMapV[index];
}
