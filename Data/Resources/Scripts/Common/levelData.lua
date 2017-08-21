--Stores data for other entities and will likely be queried by parent state



local container = {}
function container.NewLevelData(baseclass)
	local data = baseclass or {}

	function data.Initialize()
		data.depth		= data.LEngineData.depth;
		data.parent		= data.LEngineData.parent;
		data.EID		= data.LEngineData.entityID;

		data.data = {}
	end

	function data.SetData(key, value)
		data.data[key] = value
	end

	function data.GetData(key)
		local returnValue = data.data[key]
		return returnValue
	end


	data.EntityInterface = {
		SetData = data.SetData,
		GetData = data.GetData
	}

	return data;
end

return container.NewLevelData;
