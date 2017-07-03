function mockHMAP()
	local hmap = {}
	hmap.maxHeight = 16
	hmap.maxRange = 15
	hmap.angleH = 0
	hmap.angleV = 0
	
	for i=0, hmap.maxRange do
		hmap[i] = {}
		hmap[i].h = hmap.maxHeight
		hmap[i].v = hmap.maxHeight
	end

	function hmap.ValidateRange(self, index)
		if index < 0 or index > self.maxRange then error( index .. " HMAP index out of range") end
	end

	function hmap.GetHeightMapH(self, index)
		self:ValidateRange(index)
		return self[index].h
	end

	function hmap.GetHeightMapV(self, index)
		self:ValidateRange(index)
		return self[index].v
	end

	return hmap	
end

return mockHMAP()
