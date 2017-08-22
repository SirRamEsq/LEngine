local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewState(baseclass)
	local state = {} --ignore baseClass
	state.data = {}

	function state.Initialize()
	end

	function state.Update()
	end

	--State will be used to store data across multiple map loads
	function state.GetData(key)
		return state.data[key]
	end

	function state.SetData(key, value)
		state.data[key] = value
	end

	state.EntityInterface = {
		GetData = state.GetData,
		SetData = state.SetData
	}

	return state;
end

return container.NewState;
