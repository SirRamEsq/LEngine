local Coin = {}
result, collectableFlags = pcall(loadfile(utilityPath .. "/collectableFlags.lua", _ENV))

function Coin.new(base)
	local coin = base or {}

	--will return what coin type this is
	function coin.Collect()
		return collectableFlags.Coin
	end

	coin.EntityInterface = {
	  IsCollectable	  = function () return coin.Collect();  end,
	}

	return coin;
end

return Coin.new
