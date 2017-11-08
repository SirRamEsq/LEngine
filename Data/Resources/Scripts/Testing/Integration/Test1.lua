local result=0;

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}

	state.TESTS = {}

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

	function state.Test1()
		CPP.testing:AssertEqual(1,1)
		CPP.testing:AssertNotEqual(0,1)
	end

	--Run after every Test
	function state.Teardown()

	end


	state.TESTS[0]=state.Test1


	return state;
end

return container.NewState;
