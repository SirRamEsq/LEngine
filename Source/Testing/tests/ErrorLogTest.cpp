#include "../catch.hpp"
//#include "../../Engine/IniHandler.h"
#include "../../Engine/Errorlog.h"

TEST_CASE( "ErrorLog can write to default file", "[errorlog]" ) {
    ErrorLog::noThrow = false;
    //Using C11 Lambda
    REQUIRE_NOTHROW([&](){
        K_Log.Write("TEST");
        K_Log.Write("Error1", Log::SEVERITY::FATAL);
        K_Log.Write("Error2", Log::SEVERITY::ERROR);
        K_Log.Write("Error3", Log::SEVERITY::WARN);
        K_Log.Write("Error4", Log::SEVERITY::INFO);
        K_Log.Write("Error5", Log::SEVERITY::DEBUG);
        K_Log.Write("Error6", Log::SEVERITY::TRACE);
    }
    ());
    ErrorLog::noThrow = true;
}

TEST_CASE( "ErrorLog can write to custom file name", "[errorlog]" ) {
    ErrorLog::noThrow = false;
    std::string errorlog2 = "log2";
    //Using C11 Lambda
    REQUIRE_NOTHROW([&](){
        K_Log.Write("TEST", errorlog2);
        K_Log.Write("Error1", Log::SEVERITY::FATAL, errorlog2);
        K_Log.Write("Error2", Log::SEVERITY::ERROR, errorlog2);
        K_Log.Write("Error3", Log::SEVERITY::WARN, errorlog2);
        K_Log.Write("Error4", Log::SEVERITY::INFO, errorlog2);
        K_Log.Write("Error5", Log::SEVERITY::DEBUG, errorlog2);
        K_Log.Write("Error6", Log::SEVERITY::TRACE, errorlog2);
    }
    ());
    ErrorLog::noThrow = true;
}

TEST_CASE( "ErrorLog throws when given unusable file name", "[errorlog]" ) {
    std::string willThrowLogName = "loglkjanrgf983u9546][\\///2";

    //will throw
    ErrorLog::noThrow = false;
    REQUIRE_THROWS_AS(  K_Log.Write("TEST", willThrowLogName), ErrorLog::Exception );

    //will fail silently
    ErrorLog::noThrow = true;
    REQUIRE_NOTHROW(  K_Log.Write("TEST", willThrowLogName) );
}
