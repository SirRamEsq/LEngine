function NewCamera(baseclass)
	local camera = baseclass or {}

	--Ignore baseclass
	camera= {};
	camera.x= {};
	camera.y= {};
	camera.w=480;
	camera.h=320;
	camera.saveIndex=0;
	camera.loadIndex=0;
	camera.rect={}
	camera.rect.x=-16;
	camera.rect.y= -8;
	camera.rect.w= 32;
	camera.rect.h= 16;
	camera.CAMERA_BUFFER=5;

	camera.localDefault = {}
	camera.localDefault.x = -200
	camera.localDefault.y = -200

	camera.blockFollow = false;

	function camera.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------
		camera.depth		= camera.LEngineData.depth;
		camera.parent		= camera.LEngineData.parent;
		camera.CPPInterface = CPP.interface
		camera.EID			= camera.LEngineData.entityID;
		camera.pos			= CPP.Coord2df(camera.localDefault.x,camera.localDefault.y);

		camera.myPositionComp=camera.CPPInterface:GetPositionComponent(camera.EID);

		--Decide how camera will follow its parent
		if(camera.blockFollow) then
			camera.myPositionComp:ChangeParent(0);
		else
			--Auto follow by setting the position comp parent
			camera.myPositionComp:ChangeParent(camera.LEngineData.parent);
			camera.myPositionComp:SetPositionLocal(camera.pos);
		end

		--recieve parent's events
		camera.CPPInterface:EventLuaObserveEntity(camera.EID, camera.parent);

		local map = camera.CPPInterface:GetMap()
		camera.mapWidth = map:GetWidthPixels()
		camera.mapHeight = map:GetHeightPixels()
	end

	function camera.Update()
		if(camera.blockFollow) then
			parentPos = camera.CPPInterface:EntityGetPositionWorldInt(camera.parent)

			parentPos.x = (math.floor(parentPos.x/camera.w) * camera.w)
			parentPos.y = (math.floor(parentPos.y/camera.h) * camera.h)

			camera.myPositionComp:SetPositionLocalInt(parentPos);
		else
			local worldPos = camera.myPositionComp:GetPositionWorldInt()
			local localPos = camera.myPositionComp:GetPositionLocalInt()
			local newPos = CPP.Coord2d(camera.localDefault.x,camera.localDefault.y)

			if (worldPos.x < 0) then
				newPos.x = localPos.x - worldPos.x
			elseif ( (worldPos.x + camera.w) > camera.mapWidth) then
				newPos.x = localPos.x - ((worldPos.x + camera.w) - camera.mapWidth)
			else
				--push camera back to localx + distance to border
				local differenceLeft = worldPos.x
				local differenceRight = worldPos.x+camera.w
				if(localPos.x < camera.localDefault.x)then
					newPos.x =	localPos.x - ((worldPos.x + camera.w) - camera.mapWidth)
					if(newPos.x>camera.localDefault.x)then newPos.x = camera.localDefault.x end
				elseif(localPos.x > camera.localDefault.x)then
					newPos.x = localPos.x - worldPos.x
					if(newPos.x<camera.localDefault.x)then newPos.x = camera.localDefault.x end
				else
					newPos.x = camera.localDefault.x
				end
			end
			if (worldPos.y < 0) then
				newPos.y = localPos.y + worldPos.y
			elseif ( (worldPos.y + camera.h) > camera.mapHeight) then
				newPos.y = localPos.y - ((worldPos.y + camera.h) - camera.mapHeight)
			else
				--push camera back to localx + distance to border
				local differenceLeft = worldPos.y
				local differenceRight = worldPos.y+camera.h
				if(localPos.y < camera.localDefault.y)then
					newPos.y =	localPos.y - ((worldPos.y + camera.h) - camera.mapHeight)
					if(newPos.y>camera.localDefault.y)then newPos.y = camera.localDefault.y end
				elseif(localPos.y > camera.localDefault.y)then
					newPos.y = localPos.y - worldPos.y
					if(newPos.y<camera.localDefault.y)then newPos.y = camera.localDefault.y end
				else
					newPos.y = camera.localDefault.y
				end
			end

			camera.myPositionComp:SetPositionLocalInt(newPos)

		end
	end

	return camera;
end

return NewCamera;
