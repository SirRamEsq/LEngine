local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewState(baseclass)
	local state = {} --ignore baseClass
	state.data = {}

	-----------------
	--CPP Functions--
	-----------------
	function state.Initialize() end

	function state.Update() end

	--This function is called resuming after the state above it is popped off the stack
	function state.Resume() end

	--Not a destructor, is only called when this state is popped off the stack
	function state.Close() end

	-----------------
	--Lua Functions--
	-----------------
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
