function newCollisionSystemMock()
	local mock={};
	mock.boxes={}
	
	function mock.GetBox(self, id)
		local box = self.boxes[id]
		if (box == nil)then
			error("ID doesn't exist")
		end
		return box
	end

	function mock.AddCollisionBox(self, rectangle, id)
		self.boxes[id]=rectangle
	end

	function mock.CheckForEntities(self, id)
		local box = self:GetBox(id) 
		box.entityCheck = true
	end

	function mock.CheckForTiles(self, id)
		local box = self:GetBox(id)
		box.tileCheck = true
	end

	function mock.SetPrimaryCollisionBox(self, id)
		local box = self:GetBox(id)
		box.primary= true
	end

	function mock.ChangeWidth(self, id, w)
		local box = self:GetBox(id)
		box.w = w
	end

	function mock.ChangeHeight(self, id, h)
		local box = self:GetBox(id)
		box.h = h
	end

	return mock
end
return newCollisionSystemMock()
