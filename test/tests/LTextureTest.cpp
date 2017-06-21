#include "../catch.hpp"
#include "../../Source/Kernel.h"

TEST_CASE( "Can Load and Bind Texture", "[resources][texture]" ) {
    Kernel::Inst();

    auto itemCount = Kernel::rscTexMan.GetSize();
    REQUIRE(itemCount == 0);

    auto resource = Kernel::rscTexMan.GetLoadItem("TEX1", "emerald.png");
    REQUIRE(resource != NULL);

    auto name = resource->GetName();
    REQUIRE(name == "emerald.png");

    auto width = resource->GetWidth();
    REQUIRE(width == 48);

    auto height = resource->GetHeight();
    REQUIRE(height == 32);

    resource->Bind();
    auto id = resource->GetOpenGLID();
    REQUIRE(id != 0);

    itemCount = Kernel::rscTexMan.GetSize();
    REQUIRE(itemCount == 1);

    Kernel::Close();

    itemCount = Kernel::rscTexMan.GetSize();
    REQUIRE(itemCount == 0);
}
