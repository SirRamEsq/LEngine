#include "../catch.hpp"
//#include "../../Engine/IniHandler.h"
#include "../../Engine/Errorlog.h"

TEST_CASE( "ErrorLog can write to default file", "[errorlog]" ) {
    ErrorLog::noThrow = false;
    //Using C11 Lambda
    REQUIRE_NOTHROW([&](){
        ErrorLog::WriteToFile("TEST");
        ErrorLog::WriteToFile("Error1", ErrorLog::SEVERITY::FATAL);
        ErrorLog::WriteToFile("Error2", ErrorLog::SEVERITY::ERROR);
        ErrorLog::WriteToFile("Error3", ErrorLog::SEVERITY::WARN);
        ErrorLog::WriteToFile("Error4", ErrorLog::SEVERITY::INFO);
        ErrorLog::WriteToFile("Error5", ErrorLog::SEVERITY::DEBUG);
        ErrorLog::WriteToFile("Error6", ErrorLog::SEVERITY::TRACE);
    }
    ());
    ErrorLog::noThrow = true;
}

TEST_CASE( "ErrorLog can write to custom file name", "[errorlog]" ) {
    ErrorLog::noThrow = false;
    std::string errorlog2 = "log2";
    //Using C11 Lambda
    REQUIRE_NOTHROW([&](){
        ErrorLog::WriteToFile("TEST", errorlog2);
        ErrorLog::WriteToFile("Error1", ErrorLog::SEVERITY::FATAL, errorlog2);
        ErrorLog::WriteToFile("Error2", ErrorLog::SEVERITY::ERROR, errorlog2);
        ErrorLog::WriteToFile("Error3", ErrorLog::SEVERITY::WARN, errorlog2);
        ErrorLog::WriteToFile("Error4", ErrorLog::SEVERITY::INFO, errorlog2);
        ErrorLog::WriteToFile("Error5", ErrorLog::SEVERITY::DEBUG, errorlog2);
        ErrorLog::WriteToFile("Error6", ErrorLog::SEVERITY::TRACE, errorlog2);
    }
    ());
    ErrorLog::noThrow = true;
}

TEST_CASE( "ErrorLog throws when given unusable file name", "[errorlog]" ) {
    std::string willThrowLogName = "loglkjanrgf983u9546][\\///2";

    //will throw
    ErrorLog::noThrow = false;
    REQUIRE_THROWS_AS(  ErrorLog::WriteToFile("TEST", willThrowLogName), ErrorLog::Exception );

    //will fail silently
    ErrorLog::noThrow = true;
    REQUIRE_NOTHROW(  ErrorLog::WriteToFile("TEST", willThrowLogName) );
}