function mockTileCollisionPacket()
	local packet = {}
	
	packet.id	= 20
	packet.x	= 2
	packet.y	= 2
	packet.hmap = nil
	packet.layer= nil

	function packet.GetID(self)
		return self.id
	end

	function packet.GetHmap(self)
		return self.hmap
	end

	function packet.GetX(self)
		return self.x
	end

	function packet.GetY(self)
		return self.y
	end

	function packet.GetLayer(self)
		return self.layer
	end

	return packet
end

return mockTileCollisionPacket()
