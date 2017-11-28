--[[
--TO USE:
--Make sure class.C.WIDTH and HEIGHT are set before calling Init
--
--Can override the following functions before Init is called
--	class.OnEntityCollision
--]]
local container = {}

function container.new(base)
	local class = base or {}
	local result
	class.entityCollision = {}
	class.entityCollision.primary = {}
	class.entityCollision.primary.box = nil
	class.entityCollision.primary.ID = 0
	class.entityCollision.primary.order = 200

	class.InitFunctions = class.InitFunctions or {}

	function printTable(t)
		local retString = ""
		for k,v in pairs(t)do
			retString = retString .. "\r\n ["..tostring(k).."] = "..tostring(v)
		end
		return retString
	end


	function tileInit()
		local eid = class.LEngineData.entityID
		local collision = CPP.interface:GetCollisionComponent(eid)

		--CPP.interface:LogWarn(eid, tostring(eid) .. " EID")
		--CPP.interface:LogWarn(eid, "Type: " .. printTable(class) )
		--CPP.interface:LogWarn(eid, "Type.entityCollision: " .. printTable(class.entityCollision) )
		--CPP.interface:LogWarn(eid, "Type.UpdateFunctions: " .. printTable(class.UpdateFunctions) )
		--CPP.interface:LogWarn(eid, "Type.InitFunctions: " .. printTable(class.InitFunctions) )
		--Primary collision
		class.entityCollision.primary.box = CPP.Rect(0, 0, class.C.WIDTH, class.C.HEIGHT)
		local primaryBox = class.entityCollision.primary.box
		local primaryOrder = class.entityCollision.primary.order

		class.entityCollision.primary.ID = collision:AddCollisionBox(primaryBox, primaryOrder)

		local primaryID = class.entityCollision.primary.ID
		collision:CheckForEntities(primaryID)
		collision:SetPrimaryCollisionBox(primaryID)
	end

	function class.OnTileCollision(packet)
		local position = CPP.interface:GetPositionComponent(eid)
		local absolutePos = position:GetPositionWorld():Round()
		local speed = position:GetMovement():Round()
		class.tileCollision.OnTileCollision(packet, speed.x, speed.y, absolutePos.x, absolutePos.y)
	end

	function class.OnEntityCollision(entityID, packet)
		local eid = class.LEngineData.entityID
		--CPP.interface:LogWarn(eid, tostring(eid) .. " Needs to have OnEntityCollision defined")
	end

	--Add to sequence of init functions to call
	table.insert(class.InitFunctions, tileInit)

	return class;
end

return container.new
