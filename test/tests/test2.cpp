#include "catch.hpp"
#include "../../Source/IniHandler.h"
#include "../../Source/Errorlog.h"

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

TEST_CASE( "ErrorLog can write to file", "[errorlog]" ) {
    ErrorLog::WriteToFile("TEST");
    ErrorLog::WriteToFile("Error1", ErrorLog::SEVERITY::FATAL);
    ErrorLog::WriteToFile("Error2", ErrorLog::SEVERITY::ERROR);
    ErrorLog::WriteToFile("Error3", ErrorLog::SEVERITY::WARN);
    ErrorLog::WriteToFile("Error4", ErrorLog::SEVERITY::INFO);
    ErrorLog::WriteToFile("Error5", ErrorLog::SEVERITY::DEBUG);
    ErrorLog::WriteToFile("Error6", ErrorLog::SEVERITY::TRACE);

    std::string errorlog2 = "log2";
    ErrorLog::WriteToFile("TEST", errorlog2);
    ErrorLog::WriteToFile("Error1", ErrorLog::SEVERITY::FATAL, errorlog2);
    ErrorLog::WriteToFile("Error2", ErrorLog::SEVERITY::ERROR, errorlog2);
    ErrorLog::WriteToFile("Error3", ErrorLog::SEVERITY::WARN, errorlog2);
    ErrorLog::WriteToFile("Error4", ErrorLog::SEVERITY::INFO, errorlog2);
    ErrorLog::WriteToFile("Error5", ErrorLog::SEVERITY::DEBUG, errorlog2);
    ErrorLog::WriteToFile("Error6", ErrorLog::SEVERITY::TRACE, errorlog2);

    std::string willThrowLogName = "loglkjanrgf983u9546][\\///2";
    REQUIRE_THROWS_AS(  ErrorLog::WriteToFile("TEST", willThrowLogName), ErrorLog::Exception );
}

SCENARIO( "vectors can be sized and resized", "[vector]" ) {

    GIVEN( "A vector with some items" ) {
        std::vector<int> v( 5 );

        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 5 );

        WHEN( "the size is increased" ) {
            v.resize( 10 );

            THEN( "the size and capacity change" ) {
                REQUIRE( v.size() == 10 );
                REQUIRE( v.capacity() >= 10 );
            }
        }
        WHEN( "the size is reduced" ) {
            v.resize( 0 );

            THEN( "the size changes but not capacity" ) {
                REQUIRE( v.size() == 0 );
                REQUIRE( v.capacity() >= 5 );
            }
        }
        WHEN( "more capacity is reserved" ) {
            v.reserve( 10 );

            THEN( "the capacity changes but not the size" ) {
                REQUIRE( v.size() == 5 );
                REQUIRE( v.capacity() >= 10 );
            }
        }
        WHEN( "less capacity is reserved" ) {
            v.reserve( 0 );

            THEN( "neither size nor capacity are changed" ) {
                REQUIRE( v.size() == 5 );
                REQUIRE( v.capacity() >= 5 );
            }
        }
    }
}
