function NewChangeMap(baseclass)
	local cMap= baseclass or {}

	function cMap.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------

		cMap.depth          = cMap.LEngineData.depth;
		cMap.parentEID      = cMap.LEngineData.parentEID;
		cMap.EID            = cMap.LEngineData.entityID;
		local EID = cMap.EID

		cMap.spriteComp = CPP.interface:GetSpriteComponent    (EID);
		cMap.posComp	= CPP.interface:GetPositionComponent  (EID);
		cMap.colComp	= CPP.interface:GetCollisionComponent  (EID);

		cMap.spriteName = cMap.LEngineData.InitializationTable["sprite"] or "vortex.xml"
		cMap.animationName = cMap.LEngineData.InitializationTable["animation"] or ""
		cMap.rotationSpeed = cMap.LEngineData.InitializationTable["rotationSpeed"] or 2.5
		cMap.animationSpeed = cMap.LEngineData.InitializationTable["animationSpeed"] or 0.1
		cMap.map = cMap.LEngineData.InitializationTable["map"] or "Hub.tmx"

		----------------
		--Sprite setup--
		----------------
		cMap.sprite = CPP.interface:LoadSprite(cMap.spriteName);
		if(cMap.sprite==nil) then
			CPP.interface:LogError("sprite is NIL");
		end
		cMap.sprWidth = cMap.sprite:Width()
		cMap.sprHeight = cMap.sprite:Height()

		--Logical origin is as at the top left; (0,0) is top left
		--Renderable origin is at center;       (-width/2, -width/2) is top left
		--To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
		cMap.spriteID = cMap.spriteComp:AddSprite(cMap.sprite, cMap.depth, 0, 0);
		cMap.spriteComp:SetAnimation(cMap.spriteID, cMap.animationName);
		cMap.spriteComp:SetAnimationSpeed(cMap.spriteID, cMap.animationSpeed);
		cMap.spriteComp:SetRotation (cMap.spriteID, 0);
		cMap.rotation = 0

		-------------------
		--Collision setup--
		-------------------
		cMap.colbox = CPP.Rect(0, 0, cMap.sprWidth, cMap.sprHeight)
		cMap.colboxID = 1
		cMap.colComp:AddCollisionBox(cMap.colbox, cMap.colboxID, 0)
		cMap.colComp:SetPrimaryCollisionBox(cMap.colboxID)
		cMap.colComp:CheckForEntities(cMap.colboxID)
	end

	function cMap.Update()
		if cMap.rotationSpeed ~= 0 then
			cMap.rotation = cMap.rotation + cMap.rotationSpeed
			cMap.spriteComp:SetRotation (cMap.spriteID, cMap.rotation);
		end
	end

	function cMap.OnEntityCollision(entityID, packet)
		cMap.rotation = 0
	end

	function cMap.OnLuaEvent(senderEID, eventString)

	end

	function cMap.Activate()
		CPP.interface:LoadMap(cMap.map, 0)
	end

	cMap.EntityInterface = cMap.EntityInterface or {}
	cMap.EntityInterface.IsSolid     = function ()   return false; end
	cMap.EntityInterface.Activate     = function ()   cMap.Activate(); end

	return cMap
end

return NewChangeMap;
