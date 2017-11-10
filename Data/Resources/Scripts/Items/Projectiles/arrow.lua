local container = {}
function container.NewArrow(baseclass)
	local arrow = baseclass or {}
	--Constants
	arrow.WIDTH=16;
	arrow.HEIGHT=16;

	arrow.MAX_SPEED_X=5;
	arrow.xspd=0;

	arrow.DIRECTION_LEFT=-1;
	arrow.DIRECTION_RIGHT=1;
	arrow.DIRECTION=0;
	arrow.totalDistance=0

	arrow.CBOX_PRIME_ID=0;

	arrow.CBOX_X=0;
	arrow.CBOX_Y=0;
	arrow.CBOX_W=16;
	arrow.CBOX_H=16;

	arrow.T_CBOX_LEFT=nil;
	arrow.T_CBOX_RIGHT=nil;
	arrow.T_CBOX_ID=1;

	arrow.cboxPrimary=nil;


--Variables
	arrow.imgSpeed=.25;

	--C++ Interfacing
		arrow.CPPInterface=nil;
		arrow.mySprite=nil;
		arrow.mySpriteID=0;
		arrow.mySpriteComp=nil;
		arrow.myColComp=nil;
		arrow.myPositionComp=nil;
		arrow.EID=0;
		arrow.depth=0;
		arrow.parentEID=0;


function arrow.Initialize()
	-----------------------
	--C++ Interface setup--
	-----------------------

	arrow.depth				= arrow.LEngineData.depth;
	arrow.parentEID			= arrow.LEngineData.parentEID;
	arrow.CPPInterface 		= CPP.interface
	arrow.EID				= arrow.LEngineData.entityID;
	local EID = arrow.EID

	arrow.dir=arrow.LEngineData.InitializationTable.direction or "right"
	if arrow.dir == "left" then arrow.DIRECTION=arrow.DIRECTION_LEFT else arrow.DIRECTION=arrow.DIRECTION_RIGHT end

	arrow.shooterEID=arrow.LEngineData.InitializationTable.shooterEID or 0

	arrow.mySpriteComp	 = arrow.CPPInterface:GetSpriteComponent		(EID);
	arrow.myColComp			 = arrow.CPPInterface:GetCollisionComponent (EID);
	arrow.myPositionComp = arrow.CPPInterface:GetPositionComponent	(EID);

	----------------
	--Sprite setup--
	----------------

	arrow.mySprite = arrow.CPPInterface:LoadSprite("SpriteArrow.xml");
	if(arrow.mySprite==nil) then
		arrow.CPPInterface:LogError("sprite is NIL");
	end

	--Logical origin is as at the top left; (0,0) is top left
	--Renderable origin is at center;				(-width/2, -width/2) is top left
	--To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
	arrow.mySpriteID=arrow.mySpriteComp:AddSprite(arrow.mySprite, arrow.depth, (arrow.WIDTH/2), (arrow.HEIGHT/2)+1);
	arrow.mySpriteComp:SetAnimation(arrow.mySpriteID, "Fire");
	arrow.mySpriteComp:SetRotation(arrow.mySpriteID, 0);

	arrow.cboxPrimary = CPP.Rect(arrow.CBOX_X, arrow.CBOX_Y, arrow.CBOX_W,	arrow.CBOX_H);
	arrow.myColComp:AddCollisionBox(arrow.cboxPrimary, arrow.CBOX_PRIME_ID, 0);
	arrow.myColComp:CheckForEntities(arrow.CBOX_PRIME_ID);

	arrow.T_CBOX_LEFT  = CPP.Rect(0,7,3,3);
	arrow.T_CBOX_RIGHT = CPP.Rect(15,7,3,3);

	if arrow.DIRECTION==arrow.DIRECTION_LEFT then arrow.myColComp:AddCollisionBox(arrow.T_CBOX_LEFT, arrow.T_CBOX_ID, 0);
	else arrow.myColComp:AddCollisionBox(arrow.T_CBOX_RIGHT, arrow.T_CBOX_ID, 0); end

	arrow.myColComp:CheckForTiles		(arrow.T_CBOX_ID);
	arrow.myColComp:CheckForEntities(arrow.T_CBOX_ID);

	arrow.myColComp:SetPrimaryCollisionBox(arrow.CBOX_PRIME_ID, false);
	arrow.myColComp:SetName(arrow.LEngineData.name)
	arrow.myColComp:SetType(arrow.LEngineData.objType);

	arrow.totalDistance=0;
	--Allow tiled to overwrite;
	if arrow.LEngineData.InitializationTable.Solid == nil then arrow.LEngineData.InitializationTable.Solid=true; end

	arrow.CPPInterface:EventLuaObserveEntity(arrow.EID, arrow.shooterEID);

end
arrow.globalCol=0;
arrow.time=0;
function arrow.Update()
	arrow.time = arrow.time + 1
	arrow.totalDistance= arrow.totalDistance + arrow.MAX_SPEED_X;
	if arrow.totalDistance > (1000) then arrow.CPPInterface:EntityDelete(arrow.EID); end
	if arrow.time > (160) then arrow.CPPInterface:EntityDelete(arrow.EID); end

	arrow.updateVec= CPP.Coord2df(arrow.MAX_SPEED_X * arrow.DIRECTION,0);
	if arrow.updateVec.x < 0 then arrow.mySpriteComp:SetScalingX(arrow.mySpriteID, -1) else arrow.mySpriteComp:SetScalingX(arrow.mySpriteID, 1) end

	arrow.myPositionComp:SetMovement(arrow.updateVec);
end


function arrow.OnEntityCollision(entityID, packet)
	local boxID=packet:GetID();
	local name= packet:GetName();
	local ttype= packet:GetType();
	arrow.globalCol=arrow.globalCol+1;

	if(boxID==arrow.T_CBOX_ID)then
		local result = arrow.CPPInterface:EntityGetInterface(entityID).Attack(1); --if true is returned, delete entity
		if(result == true) then
			arrow.CPPInterface:EntityDelete(arrow.EID);
		end
	end
end

function arrow.OnTileCollision(packet)
	--arrow.CPPInterface:EventLuaBroadcastEvent(arrow.EID, "ARROW_HIT");
	arrow.CPPInterface:EntityDelete(arrow.EID);
end

arrow.EntityInterface = arrow.EntityInterface or {}
arrow.EntityInterface.IsSolid		= function ()				return true; end

return arrow
end

return container.NewArrow;
