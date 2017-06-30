function newCollisionSystemMock()
	local mock={};
	mock.boxes={}
	
	function mock.AddCollisionBox(self, rectangle, id)
		self.boxes[id]=rectangle
	end

	function mock.CheckForEntities(self, id)
		local box = self.boxes[id]
		if (box == nil)then
			error("ID doesn't exist")
		end
		box.entityCheck = true
	end

	function mock.SetPrimaryCollisionBox(self, id)
		local box = self.boxes[id]
		if (box == nil)then
			error("ID doesn't exist")
		end
		box.primary= true
	end

	return mock
end
return newCollisionSystemMock()
