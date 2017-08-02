local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewState(baseclass)
	local state = {} --ignore baseClass

	function state.Initialize()
	end

	function state.Update()
	end

	return state;
end

return container.NewState;
