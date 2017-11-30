function NewYellowGrunt(baseclass)
	local yellowGrunt = baseclass or {}
	--Constants
	yellowGrunt.WIDTH=16;
	yellowGrunt.HEIGHT=16;
	yellowGrunt.GRAVITY=0.21875;
	yellowGrunt.JUMPHEIGHT=-5;

	yellowGrunt.ySpeed = 0;
	yellowGrunt.xSpeed = 0;
	yellowGrunt.xSpeedMax=2;
	yellowGrunt.xSpeedIncrement=.2
	yellowGrunt.negative=false;

	yellowGrunt.DIRECTION_LEFT="left";
	yellowGrunt.DIRECTION_RIGHT="right";
	yellowGrunt.DIRECTION=0;

	yellowGrunt.xPos = 0
	yellowGrunt.yPos = 0

	yellowGrunt.CBOX_PRIME_ID=0;

	yellowGrunt.CBOX_X=0;
	yellowGrunt.CBOX_Y=0;
	yellowGrunt.CBOX_W=16;
	yellowGrunt.CBOX_H=16;

	yellowGrunt.RELOAD_TIME=60;
	yellowGrunt.SHOOT_TIME=20;

	yellowGrunt.cboxPrimary=nil;
	local result=0;
	result, yellowGrunt.timing    = pcall(loadfile(utilityPath .. "/timing.lua", _ENV))


	yellowGrunt.ALARM_NAME1=1;
	yellowGrunt.ALARM_NAME2=2;

	yellowGrunt.arrowsFired=0;

	--C++ Interfacing
	yellowGrunt.CPPInterface=nil;
	yellowGrunt.mySprite=nil;
	yellowGrunt.mySpriteComp=nil;
	yellowGrunt.mySpriteID=0;
	yellowGrunt.myColComp=nil;
	yellowGrunt.myPositionComp=nil;
	yellowGrunt.EID=0;
	yellowGrunt.depth=0;
	yellowGrunt.parentEID=0;


	function yellowGrunt.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------

		yellowGrunt.depth        = yellowGrunt.LEngineData.depth;
		yellowGrunt.parentEID       = yellowGrunt.LEngineData.parentEID;
		yellowGrunt.CPPInterface = CPP.interface
		yellowGrunt.EID          = yellowGrunt.LEngineData.entityID;

		yellowGrunt.mySpriteComp   = yellowGrunt.CPPInterface:GetSpriteComponent    (yellowGrunt.EID);
		yellowGrunt.myColComp      = yellowGrunt.CPPInterface:GetCollisionComponent (yellowGrunt.EID);
		yellowGrunt.myPositionComp = yellowGrunt.CPPInterface:GetPositionComponent  (yellowGrunt.EID);

		----------------
		--Sprite setup--
		----------------
		yellowGrunt.mySprite = yellowGrunt.CPPInterface:LoadSprite("SpriteYellowGrunt.xml");
		if(yellowGrunt.mySprite==nil) then
			yellowGrunt.CPPInterface:LogError("sprite is NIL");
		end

		--Logical origin is as at the top left; (0,0) is top left
		--Renderable origin is at center;       (-width/2, -width/2) is top left
		--To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
		yellowGrunt.mySpriteID = yellowGrunt.mySpriteComp:AddSprite(yellowGrunt.mySprite, yellowGrunt.depth,0,0)
		yellowGrunt.mySpriteComp:SetAnimation(yellowGrunt.mySpriteID, "Stand");
		yellowGrunt.mySpriteComp:SetRotation(yellowGrunt.mySpriteID, 0);

		yellowGrunt.dir=yellowGrunt.LEngineData.InitializationTable.direction or "right"
		if yellowGrunt.dir == "left" then yellowGrunt.DIRECTION=yellowGrunt.DIRECTION_LEFT else yellowGrunt.DIRECTION=yellowGrunt.DIRECTION_RIGHT end

		yellowGrunt.timing:SetAlarm(yellowGrunt.ALARM_NAME1, yellowGrunt.SHOOT_TIME, yellowGrunt.OnShoot, false) --don't repeat alarms
		yellowGrunt.timing:SetAlarm(yellowGrunt.ALARM_NAME2, yellowGrunt.RELOAD_TIME, yellowGrunt.OnReload, false) --don't repeat alarms
		yellowGrunt.timing:GetAlarm(yellowGrunt.ALARM_NAME2):Disable();
	end

	function yellowGrunt.OnShoot()
		--create entity and listen to events
		local entityArrow;
		local position=yellowGrunt.myPositionComp:GetPositionWorld();
		yellowGrunt.arrowsFired=yellowGrunt.arrowsFired+1;

		local name = "";
		local scriptName = "Items/Projectiles/arrow.lua"
		entityArrow = yellowGrunt.CPPInterface:EntityNew(
			name, position.x, position.y, yellowGrunt.depth, 0, scriptName,
		{direction = yellowGrunt.DIRECTION, shooterEID = yellowGrunt.EID})

		yellowGrunt.CPPInterface:EventLuaObserveEntity(yellowGrunt.EID, entityArrow);

		--Update sprite
		yellowGrunt.mySpriteComp:SetAnimation(yellowGrunt.mySpriteID, "Shoot");

		--Update Alarm
		local alarm=yellowGrunt.timing:GetAlarm(yellowGrunt.ALARM_NAME2)
		alarm:Restart();
	end

	function yellowGrunt.OnReload()
		--Update Sprite
		yellowGrunt.mySpriteComp:SetAnimation(yellowGrunt.mySpriteID, "Stand");

		--Update Alarm
		local alarm=yellowGrunt.timing:GetAlarm(yellowGrunt.ALARM_NAME1)
		alarm:Restart();
	end

	function yellowGrunt.Update()
		yellowGrunt.timing:Update();
	end

	function yellowGrunt.OnLuaEvent(senderEID, eventString)
	end

	yellowGrunt.EntityInterface = yellowGrunt.EntityInterface or {}
	yellowGrunt.EntityInterface.CanBounce   = function ()       return true; end

	return yellowGrunt;
end

return NewYellowGrunt;
