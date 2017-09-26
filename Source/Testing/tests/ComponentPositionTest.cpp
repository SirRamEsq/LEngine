#include "../catch.hpp"
#include "../../Engine/Kernel.h"
#include "../../Engine/Components/CompScript.h"
#include "../../Engine/LuaInterface.h"
#include "../../Engine/Defines.h"
#include "../mocks/GameStateMock.h"
#include "../mocks/RSC_MapMock.h"

TEST_CASE("Test ComponentPosition and Manager", "[position]"){
	Kernel::Inst();

	auto stateManager = &K_StateMan;
	auto stateSmrtPtr = make_unique<GameStateMock>(stateManager);
	auto state = stateSmrtPtr.get();
	stateManager->PushState(std::move(stateSmrtPtr));

	Kernel::Update();

	auto posMan = state->GetComponentManagerPosition();
	auto eid = 250;
	posMan->AddComponent(eid);

	auto posComp = posMan->GetComponent(eid);
	REQUIRE(posComp != NULL);

	SECTION("Position, Velocity, Accleration are calculated correctly"){
		float startX = 32.0f;
		float startY = 32.0f;
		float xspd = 2.0f;
		float yspd = 1.0f;
		float xaccl = 0.2f;
		float yaccl = 0.1f;

		posComp->SetPositionLocal(Coord2df(startX, startY));
		posComp->SetMovement(Coord2df(xspd, yspd));
		posComp->SetAcceleration(Coord2df(xaccl, yaccl));

		posComp->Update();

		auto localPos = posComp->GetPositionLocal();
		auto velocity = posComp->GetMovement();

		REQUIRE(localPos.x == Approx(startX + xspd));
		REQUIRE(localPos.y == Approx(startY + yspd));
		REQUIRE(velocity.x == Approx(xspd + xaccl));
		REQUIRE(velocity.y == Approx(yspd + yaccl));
	}

	SECTION("Parents can be added and updated correctly"){
		auto parentEID = 210;
		posMan->AddComponent(parentEID);
		auto parent = posMan->GetComponent(parentEID);
		REQUIRE(parent != NULL);

		auto childEID = eid;
		auto child = posComp;

		child->SetParentEID(parentEID);
		auto childNode = child->GetMapNode();
		auto parentNode = parent->GetMapNode();
		REQUIRE(childNode->mParent == parentNode);
		REQUIRE(child->GetParent() == parent);

		float parentX = 16.0f;
		float parentY = 32.0f;
		float childX = 4.0f;
		float childY = 2.0f;

		parent->SetPositionLocal(Coord2df(parentX, parentY));
		child->SetPositionLocal(Coord2df(childX, childY));

		float parentXspd = 2.0f;
		float parentYspd = 1.0f;
		parent->SetMovement(Coord2df(parentXspd, parentYspd));
		child->SetMovement(Coord2df(0.0f, 0.0f));

		//parent should be updated before child
		posMan->Update();

		REQUIRE(parent->GetPositionWorld().x == Approx(parentX + parentXspd));
		REQUIRE(parent->GetPositionWorld().y == Approx(parentY + parentYspd));
		//This will be false if the child was updated before the parent
		REQUIRE(child->GetPositionWorld().x == Approx(childX + parentX + parentXspd));
		REQUIRE(child->GetPositionWorld().y == Approx(childY + parentY + parentYspd));
		//Local should be the same as its initial value, the child's position relative to its parent hasn't changed
		REQUIRE(child->GetPositionLocal().x == Approx(childX));
		REQUIRE(child->GetPositionLocal().y == Approx(childY));
	}

	SECTION("Ensure that coordinates can be translated between world and local"){
		auto parentEID = 210;
		posMan->AddComponent(parentEID);
		auto parent = posMan->GetComponent(parentEID);

		REQUIRE(parent != NULL);

		auto childEID = eid;
		auto child = posComp;

		child->SetParentEID(parentEID);
		auto childNode = child->GetMapNode();
		auto parentNode = parent->GetMapNode();
		REQUIRE(childNode->mParent == parentNode);
		REQUIRE(child->GetParent() == parent);

		float parentX = 16.0f;
		float parentY = 32.0f;
		float childX = 4.0f;
		float childY = 2.0f;

		parent->SetPositionLocal(Coord2df(parentX, parentY));
		child->SetPositionLocal(Coord2df(childX, childY));

		float parentXspd = 2.0f;
		float parentYspd = 1.0f;
		parent->SetMovement(Coord2df(parentXspd, parentYspd));
		child->SetMovement(Coord2df(0.0f, 0.0f));

		posMan->Update();

		REQUIRE(parent->GetPositionWorld().x == Approx(parentX + parentXspd));
		REQUIRE(parent->GetPositionWorld().y == Approx(parentY + parentYspd));

		//Ensure that these values differ
		REQUIRE(child->GetPositionWorld().x != Approx(child->GetPositionLocal().x));
		REQUIRE(child->GetPositionWorld().y != Approx(child->GetPositionLocal().y));

		auto translatedToLocal = child->TranslateWorldToLocal(child->GetPositionWorld());
		auto translatedToWorld = child->TranslateLocalToWorld(child->GetPositionLocal());
		REQUIRE(child->GetPositionWorld().x == Approx(translatedToWorld.x));
		REQUIRE(child->GetPositionLocal().x == Approx(translatedToLocal.x));
		REQUIRE(child->GetPositionWorld().y == Approx(translatedToWorld.y));
		REQUIRE(child->GetPositionLocal().y == Approx(translatedToLocal.y));

		//Ensure that setting the WorldPosition Works properly
		float childWorldX = 50.0f;
		float childWorldY = 50.0f;

		child->SetPositionWorld(Coord2df(childWorldX, childWorldY));
		REQUIRE(child->GetPositionWorld().x == Approx(childWorldX));
		REQUIRE(child->GetPositionWorld().y == Approx(childWorldY));
		//The local position of the child should be the difference between
		//its world position and its parent's world position
		REQUIRE(child->GetPositionLocal().x == Approx(childWorldX - parent->GetPositionWorld().x));
		REQUIRE(child->GetPositionLocal().y == Approx(childWorldY - parent->GetPositionWorld().y));
	}

	SECTION("Can Get Root Node"){
		REQUIRE(posComp->GetMapNode()->GetRootNode() == posMan->GetRootNode());
	}

	Kernel::Close();
}
