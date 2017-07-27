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

		CPP.interface:WriteError(camera.EID, "EID is " .. tostring(camera.EID))
		camera.CPPInterface:WriteError(camera.EID, "Parent is " .. tostring(camera.parent))
		camera.myPositionComp=camera.CPPInterface:GetPositionComponent(camera.EID);

		--Set parent for All Component Managers (That have been added thus far: position and script)
		--This will ensure that the parent script will be updated before this script
		--Therefore, the entity the camera is following will move before the camera does
		--This will ensure that the camera will always follow an up-to-date position and not lag behind
		camera.CPPInterface:WriteError(camera.EID, "Attempting to set Parent");
		CPP.interface:SetParent(camera.EID, camera.parent)
		camera.CPPInterface:WriteError(camera.EID, "Parent set");

		--instead of the component managing the position, the camera will manage on its own
		camera.myPositionComp:SetParent(0);
		camera.myPositionComp:SetPositionLocal(camera.pos);

		--recieve parent's events
		camera.CPPInterface:EventLuaObserveEntity(camera.EID, camera.parent);

		local map = camera.CPPInterface:GetMap()
		if(map == nil)then 
			camera.CPPInterface:WriteError(camera.EID, "Tried to get map from CPPInterface, map is nil");
		end
		camera.mapWidth = map:GetWidthPixels()
		camera.mapHeight = map:GetHeightPixels()
		camera.CPPInterface:WriteError(camera.EID, "Camera Initialized");
	end

	function camera.Update()
		if(camera.blockFollow) then
			local parentPos = camera.CPPInterface:EntityGetPositionWorld(camera.parent):Round()
			local newPos = CPP.Coord2df(0,0);	

			newPos.x = (math.floor(parentPos.x/camera.w) * camera.w)
			newPos.y = (math.floor(parentPos.y/camera.h) * camera.h)

			camera.myPositionComp:SetPositionLocal(newPos)
		else
			local parentPos = camera.CPPInterface:EntityGetPositionWorld(camera.parent):Round()
			local newPos = CPP.Coord2df(0,0);	
			--center camera on parent
			newPos.x = parentPos.x + camera.localDefault.x
			newPos.y = parentPos.y + camera.localDefault.y

			--Clamp to map borders
			if(newPos.x < 0)then newPos.x = 0 end
			if( (newPos.x + camera.w) > camera.mapWidth)then newPos.x = camera.mapWidth end
			if(newPos.y < 0)then newPos.y = 0 end
			if( (newPos.y + camera.h) > camera.mapWidth)then newPos.y = camera.mapHeight end

			camera.CPPInterface:WriteError(camera.EID, "ParentX " .. parentPos.x);
			camera.CPPInterface:WriteError(camera.EID, "ParentY " .. parentPos.y);

			camera.CPPInterface:WriteError(camera.EID, "CameraX " .. newPos.x);
			camera.CPPInterface:WriteError(camera.EID, "CameraY " .. newPos.y);
			camera.myPositionComp:SetPositionWorld(newPos)
			--[[
			local worldPos = camera.myPositionComp:GetPositionWorld()--:Round()
			local localPos = camera.myPositionComp:GetPositionLocal()--:Round()
			local newPos = CPP.Coord2df(camera.localDefault.x,camera.localDefault.y)

			if (worldPos.x < 0) then
				--Move to the right by however many pixels the camera position would be offscreen by
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

			camera.myPositionComp:SetPositionLocal(newPos)
			--]]
		end
	end

	return camera;
end

return NewCamera;
