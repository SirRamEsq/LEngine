local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}

	function state.Close()

	end

	function state.Initialize()
		CPP.interface:LoadMap("MAP1.tmx", 0)
	end

	function state.Update()
	end

	return state;
end

return container.NewState;
