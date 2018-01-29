local con = {}
function con.NewState(baseclass)
	local state = baseclass or {}

	function state.Initialize()
	end

	function state.TestRequire(testing)
		local function callback1()
			local shouldHaveValue = require("System/Testing/Integration/RequireTest.lua")
			testing:REQUIRE_NOT_EQUAL(shouldHaveValue, nil)
		end
		testing:REQUIRE_NO_THROW(callback1)

		local function callback2()
			local shouldCrash = require ("_sdfewsa/_frg_nonsneas.notluaatall")
		end
		testing:REQUIRE_THROW(callback2)
	end

	state.TESTS = {state.TestRequire}

	return state;
end

return con.NewState;
