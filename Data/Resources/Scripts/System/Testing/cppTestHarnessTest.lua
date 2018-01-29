local result=0;

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}

	--Run at instantiation
	function state.Initialize()
		state.depth		= state.LEngineData.depth;
		state.parent	= state.LEngineData.parent;
		state.EID		= state.LEngineData.entityID;
		state.updateCount = 0
	end

	--Run at every invocation of testing:Update
	function state.Update()
		state.updateCount = state.updateCount + 1
	end

	--Run Before every test
	function state.Setup(testing)
		state.setupValue = 5
		state.updateCount = 0
		--Set next map to load
		--CPP.interface:LoadMap("System/Hub.tmx", 0)

		--Load map
		testing:Update();
		testing:REQUIRE_EQUAL(state.updateCount, 1)
	end

	function state.Test1(testing)
		--should PASS
		testing:REQUIRE_EQUAL(state.setupValue, 5)

		state.setupValue = 6

		--should FAIL (fail 1)
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

		--should FAIL (fail 2)
		local pass = testing:REQUIRE_EQUAL({}, {})

		--should FAIL (fail 3)
		local pass2 = testing:REQUIRE_NOT_EQUAL(1, 1)

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

		if (not pass)and(not pass2) then
			return;
		end

		--should not be run
		testing:REQUIRE_EQUAL({},{})
	end

	--Run after every Test
	function state.Teardown(testing)
		
	end

	state.TESTS = {state.Test1, state.Test2, state.Test3}

	return state;
end

return container.NewState;
