local container = {}

function container.new(base)
	local type = base or {}
	local result
	result, type.tileCollision = pcall(loadfile(commonPath .. "/collisionSystem.lua", _ENV))

	type.InitFunctions = type.InitFunctions or {}
	type.UpdateFunctions = type.UpdateFunctions or {}

	function tileInit()
		local eid = type.LEngineData.entityID
		local collision = CPP.interface:GetCollisionComponent(eid)

		type.tileCollision.Init(type.C.WIDTH, type.C.HEIGHT, CPP.interface, collision)
		type.tileCollision.callbackFunctions.TileUp    = type.tileCollision.OnTileUp
		type.tileCollision.callbackFunctions.TileDown  = type.tileCollision.OnTileDown
		type.tileCollision.callbackFunctions.TileLeft  = type.tileCollision.OnTileLeft
		type.tileCollision.callbackFunctions.TileRight = type.tileCollision.OnTileRight
	end

	function type.tileCollision.OnTileDown(newPosition, newAngle)
		local eid = type.LEngineData.entityID
		local position = CPP.interface:GetPositionComponent(eid)
		local newPosition= position:TranslateWorldToLocal(newPosition)
		position:SetPositionLocalY(newPosition.y)
	end

	function type.tileCollision.OnTileRight(newPosition)
		local eid = type.LEngineData.entityID
		local position = CPP.interface:GetPositionComponent(eid)
		local newPosition= position:TranslateWorldToLocal(newPosition)
		position:SetPositionLocalX(newPosition.x)
	end

	function type.tileCollision.OnTileLeft(newPosition)
		local eid = type.LEngineData.entityID
		local position = CPP.interface:GetPositionComponent(eid)
		local newPosition= position:TranslateWorldToLocal(newPosition)
		position:SetPositionLocalX(newPosition.x)
	end

	function type.tileCollision.OnTileUp(newPosition)
		local eid = type.LEngineData.entityID
		local position = CPP.interface:GetPositionComponent(eid)
		local newPosition= position:TranslateWorldToLocal(newPosition)
		position:SetPositionLocalY(newPosition.y)
	end

	function tileUpdate()

	end

	table.insert(type.UpdateFunctions, tileUpdate)
	table.insert(type.InitFunctions, tileInit)

	return type;
end

return container.new
