--[[
--TO USE:
--Make sure type.C.WIDTH and HEIGHT are set before calling Init
--
--Can override the following functions before Init is called
--	type.OnEntityCollision
--]]
local container = {}

function container.new(base)
	local type = base or {}
	local result
	type.entityCollision = {}
	type.entityCollision.primary = {}
	type.entityCollision.primary.box = nil
	type.entityCollision.primary.ID = 200
	type.entityCollision.primary.order = 200

	type.InitFunctions = type.InitFunctions or {}

	function printTable(t)
		local retString = ""
		for k,v in pairs(t)do
			retString = retString .. "\r\n ["..tostring(k).."] = "..tostring(v)
		end
		return retString
	end

	function tileInit()
		local eid = type.LEngineData.entityID
		local collision = CPP.interface:GetCollisionComponent(eid)

		CPP.interface:LogWarn(eid, tostring(eid) .. " EID")
		CPP.interface:LogWarn(eid, "Type: " .. printTable(type) )
		CPP.interface:LogWarn(eid, "Type.entityCollision: " .. printTable(type.entityCollision) )
		CPP.interface:LogWarn(eid, "Type.UpdateFunctions: " .. printTable(type.UpdateFunctions) )
		CPP.interface:LogWarn(eid, "Type.InitFunctions: " .. printTable(type.InitFunctions) )
		--Primary collision
		type.entityCollision.primary.box = CPP.Rect(0, 0, type.C.WIDTH, type.C.HEIGHT)
		local primaryBox = type.entityCollision.primary.box
		local primaryID = type.entityCollision.primary.ID
		local primaryOrder = type.entityCollision.primary.order

		collision:AddCollisionBox(primaryBox, primaryID, primaryOrder)
		collision:CheckForEntities(primaryID)
		collision:SetPrimaryCollisionBox(primaryID)
	end

	function type.OnTileCollision(packet)
		local position = CPP.interface:GetPositionComponent(eid)
		local absolutePos = position:GetPositionWorld():Round()
		local speed = position:GetMovement():Round()
		type.tileCollision.OnTileCollision(packet, speed.x, speed.y, absolutePos.x, absolutePos.y)
	end

	function type.OnEntityCollision(entityID, packet)
		local eid = type.LEngineData.entityID
		CPP.interface:LogWarn(eid, tostring(eid) .. " Needs to have OnEntityCollision defined")
	end

	--Add to sequence of init functions to call
	table.insert(type.InitFunctions, tileInit)

	return type;
end

return container.new
