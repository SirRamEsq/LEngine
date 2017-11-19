local container = {}
function container.New(baseclass)
	local bat = baseclass or {}
	bat.C = bat.C or {}
	bat.C.WIDTH = 24
	bat.C.HEIGHT = 24

	local Init = function()
		local eid = bat.LEngineData.entityID
		local depth = bat.LEngineData.depth
		local spriteComp = CPP.interface:GetSpriteComponent(eid);
		local sprite = CPP.interface:LoadSprite("bat.xml");

		if( sprite==nil ) then
			CPP.interface:LogError(eid, "sprite is NIL");
		end

		local spriteID	= spriteComp:AddSprite(sprite, depth, 0, 0);

		spriteComp:SetAnimation		(spriteID, "Fly");
		spriteComp:SetAnimationSpeed(spriteID, 1);
		spriteComp:SetRotation		(spriteID, 0);
	end

	local Update = function()

	end

	function bat.OnEntityCollision(eid, desc)

	end

	bat.InitFunctions = bat.InitFunctions or {}
	bat.UpdateFunctions = bat.UpdateFunctions or {}
	table.insert(bat.InitFunctions, Init)
	table.insert(bat.UpdateFunctions, Update)

	return bat
end

return container.New
