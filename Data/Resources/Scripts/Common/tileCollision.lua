--[[
--TO USE:
--Make sure type.C.WIDTH and HEIGHT are set before calling Init
--Make sure that at some point during the update funciton
--type.tileCollision.Update(xspd,yspd) is called
--
--Can override the following functions before Init is called
--	type.tileCollision.OnTileRight
--	type.tileCollision.OnTileDown
--	type.tileCollision.OnTileLeft
--	type.tileCollision.OnTileRight
--	type.OnTileCollision
--]]
local container = {}

function container.new(base)
	local type = base or {}
	local result
	result, type.tileCollision = pcall(loadfile(utilityPath .. "/collisionSystem.lua", _ENV))

	type.InitFunctions = type.InitFunctions or {}

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

		if CPP.interface.HasSpriteComponent(eid) then
			local sprite = CPP.interface.GetSpriteComponent(eid)
			--Will rotate the first sprite loaded into the SpriteComponent
			local firstSpriteLoadedID = 0
			sprite:SetRotation(firstSpriteLoadedID, newAngle);
		end
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

	function type.OnTileCollision(packet)
		local position = CPP.interface:GetPositionComponent(eid)
		local absolutePos = position:GetPositionWorld():Round()
		local speed = position:GetMovement():Round()
		type.tileCollision.OnTileCollision(packet, speed.x, speed.y, absolutePos.x, absolutePos.y)
	end

	--Add to sequence of init functions to call
	table.insert(type.InitFunctions, tileInit)

	return type;
end

return container.new
