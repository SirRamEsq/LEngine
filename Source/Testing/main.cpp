#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do
                           // this
                           // in one cpp file
//#define CATCH_CONFIG_RUNNER
#include "main.h"
#include "catch.hpp"
/*
int main(int argc, char* argv[]){
    int result = 20;
    try{
        result = Catch::Session().run(argc,argv);
    }
    catch (std::exception e){
        std::cout << e.what();
    }

    return (result > 0xff ? result : 0xff);
}

*/
