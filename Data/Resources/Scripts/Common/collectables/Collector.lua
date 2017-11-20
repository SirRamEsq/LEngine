local Collector = {}
result, collectableFlags = pcall(loadfile(utilityPath .. "/collectableFlags.lua", _ENV))

function Collector.new(base)
	local collector = base or {}
	collector.collection = {}

	function Collector.Collect(type, itemEID)
		if collector.collection[type] == nil then
			collector.collection[type] = 0
		end
		collector.collection[type] = collector.collection[type] + 1

		CPP.interface:EntityDelete(itemEID)
	end


	return collector;
end

return Collector.new
