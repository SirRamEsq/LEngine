local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}

	function state.Initialize()
		state.depth		= state.LEngineData.depth;
		state.parent		= state.LEngineData.parent;
		state.EID		= state.LEngineData.entityID;

		CPP.interface:LoadMap("MAP1.tmx", 0)
	end

	function state.Update()
	end

	return state;
end

return container.NewState;
