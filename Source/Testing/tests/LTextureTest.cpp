#include "../../Engine/Kernel.h"
#include "../catch.hpp"

TEST_CASE("Can Load and Bind Texture", "[resources][texture]") {
  Kernel::Inst();

  auto itemCount = Kernel::rscTexMan.GetSize();
  REQUIRE(itemCount == 0);

  std::string texName = "testImage.png";
  int expectedW = 72;
  int expectedH = 72;
  auto resource = Kernel::rscTexMan.GetLoadItem(texName, texName);
  REQUIRE(resource != NULL);

  auto name = resource->GetName();
  REQUIRE(name == texName);

  auto width = resource->GetWidth();
  REQUIRE(width == expectedW);

  auto height = resource->GetHeight();
  REQUIRE(height == expectedH);

  resource->Bind();
  auto id = resource->GetOpenGLID();
  REQUIRE(id != 0);

  itemCount = Kernel::rscTexMan.GetSize();
  REQUIRE(itemCount == 1);

  Kernel::Close();

  itemCount = Kernel::rscTexMan.GetSize();
  REQUIRE(itemCount == 0);
}
