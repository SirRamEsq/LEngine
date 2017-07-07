function mockTileLayer()
	local layer = {}
	layer.tiles = {}
	layer.useHMAP = false

	function layer.GetFriction(self)
		return self.friction
	end
	
	function layer.GetTile(self, x, y)
		local tileX = self.tiles[x]
		if tileX == nil then return 0 end
		local tile = tileX[y]
		if tile == nil then return 0 end
		
		return tile
	end

	function layer.IsDestructible(self)
		return false;
	end

	function layer.UsesHMaps(self)
		return self.useHMAP
	end

	return layer
end

return mockTileLayer()
