local result=0;

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}

	--Run at instantiation
	function state.Init()
		state.depth		= state.LEngineData.depth;
		state.parent	= state.LEngineData.parent;
		state.EID		= state.LEngineData.entityID;
	end

	--Run Before every test
	function state.Setup(testing)
		state.setupValue = 5
		--Set next map to load
		CPP.interface:LoadMap("Hub.tmx", 0)
		--Load map
		testing:Update();
	end

	function state.Test1(testing)
		--should PASS
		testing:REQUIRE_EQUAL(state.setupValue, 5)

		state.setupValue = 6
		testing:Error("Failure")
	end

	function state.Test2(testing)
		--should PASS
		testing:REQUIRE_EQUAL(state.setupValue, 5)

		state.setupValue = 7
	end

	function state.Test3(testing)
		--should PASS
		testing:REQUIRE_EQUAL(state.setupValue, 5)

		--should FAIL
		local pass = testing:REQUIRE_EQUAL({}, {})

		--should PASS
		testing:REQUIRE_EQUAL(1, 1)
		testing:REQUIRE_EQUAL("", "")
		testing:REQUIRE_EQUAL("asdf", "asdf")
		testing:REQUIRE_EQUAL(12.123, 12.123)
		testing:REQUIRE_EQUAL(true, true)
		testing:REQUIRE_EQUAL(false, false)
		testing:REQUIRE_EQUAL(nil, nil)
		testing:REQUIRE_EQUAL(state.Test2, state.Test2)

		testing:REQUIRE_NOT_EQUAL(1, 2)
		testing:REQUIRE_NOT_EQUAL(2,"2")
		testing:REQUIRE_NOT_EQUAL({},{})

		if (not pass) then
			return;
		end

		--should not be run
		testing:REQUIRE_EQUAL({},{})
	end

	--Run after every Test
	function state.Teardown(testing)
		local LAYER_NAME = "CLIMB"
		local currentMap = CPP.interface:GetMap()
		local LAYER

		LAYER = currentMap:GetTileLayer("oi8ujhn879546")
		testing:REQUIRE_EQUAL(LAYER, nil)

		LAYER = currentMap:GetTileLayer(LAYER_NAME)
		testing:REQUIRE_NOT_EQUAL(LAYER, nil)
	end

	state.TESTS = {state.Test1, state.Test2, state.Test3}

	return state;
end

return container.NewState;
