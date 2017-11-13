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
	end

	function state.Test3(testing)

	end
	--Run after every Test
	function state.Teardown()

	end

	state.TESTS = {state.Test3}

	return state;
end

return container.NewState;
