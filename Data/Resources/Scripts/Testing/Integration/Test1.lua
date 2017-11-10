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
	function state.Setup()
		CPP.interface:LoadMap("Hub.tmx", 0)
	end

	function state.Test1(testing)
		--testing:Assert("Failure")
	end

	function state.Test2(testing)
	end

	function state.Test3(testing)
		--testing:Assert("Not really a failure")
	end
	--Run after every Test
	function state.Teardown()

	end

	state.TESTS = {state.Test1, state.Test2, state.Test3}

	return state;
end

return container.NewState;