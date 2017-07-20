--DEBUG
--package.path = package.path .. ";/opt/zbstudio/lualibs/mobdebug/?.lua"
--local mobdebug=require('mobdebug');

--[[
Implemented
	Platform Riding
	Roll
	Long Jump
	Wall Slide
	Wall Jump

To Implement
	KLONOA style enemy/item grab
	side jump / skid jump ala Mario 64


Each map will have challenges
	pink - harm no ememies
	Green - Collect 7 emeralds
	Grey - Collect Nothing
	Blue - Time
	Red - Use no special abilities (roll)

]]--

function NewHeor(baseclass)
	local heor = baseclass or {}

	--Constants
	heor.c = {}
	heor.c.TILEWIDTH=16;
	heor.c.TILEHEIGHT=16;

	heor.c.MAXSPEED=16;
	heor.c.MAXHEALTH=10;

	heor.c.WIDTH=18; --WIDTH of object sprite
	heor.c.HEIGHT=32; --HEIGHT of object sprite

	heor.c.GRAVITY=0.21875;
	heor.c.SLOPE_GRAVITY=0.15
	heor.c.JUMPHEIGHT=-6;
	heor.c.ACCELERATION=.046875*2;
	heor.c.ACCELERATION_AIR= heor.c.ACCELERATION;
	heor.c.DEACCELERATION=1;
	heor.c.ACCELERATION_TOP=4; --Max Speed Heor can acheive through normal acceleration
	heor.c.ROLL_SPEED= heor.c.ACCELERATION_TOP+1;
	heor.c.ROLL_TIMER=15;
	heor.c.ROLL_COOLDOWN=20;
	heor.roll_timer=0;

	heor.c.FRICTION_AIR=.1;
	heor.c.FRICTION_MODIFER=0.46875*2; --The friction of a tile is multiplied by this constant to get the actual friction value
	heor.c.MAX_STEPUP_PIXELS = 12;

	heor.c.WALLJUMP_LOCK=10;
	heor.c.WALLJUMP_XSPD=5;
	heor.c.WALLJUMP_YSPD=5;
	heor.c.WALLJUMP_GRAVITY=heor.c.GRAVITY/4;
	heor.c.WALLJUMP_YSPD_MAX=2;

	heor.c.STATE_NORMAL			= 0
	heor.c.STATE_ROLL				= 1
	heor.c.STATE_GRABLEDGE	= 2
	heor.c.STATE_WALLSLIDE	= 3
	heor.c.STATE_CLIMB			= 4
	heor.c.STATE_GRAB				= 5

	heor.c.FACING_LEFT=-1;
	heor.c.FACING_RIGHT=1;

	heor.c.KNOCKBACK_SPEED_X=2.5;
	heor.c.KNOCKBACK_SPEED_Y=-3;


--Variables
	--General Movement Variables
		heor.health=heor.c.MAXHEALTH;
		heor.coinCount=0;
		heor.coinText=nil;
		heor.xspd=0;
		heor.yspd=0;
		heor.groundSpeed=0;
		heor.prevGroundSpeed=0; -- Ground speed of previous frame
		heor.tileFriction=0.046875;
		heor.inputLock=false;
		heor.lockTimer=0;

		heor.attackLock=false --character locked from being hit
		heor.angle=0;
		heor.angleSigned=0;
		heor.facingDir=heor.c.FACING_RIGHT;

		heor.ropeSwing=nil;

		heor.externalUseWhip=false;
		heor.externalUseWhipInterrupt=false;

		heor.currentState= heor.c.STATE_NORMAL;

	--CLIMBING
		heor.climb = {}
		heor.climb.SPEED = 2
		heor.climb.LAYER = nil
		heor.climb.LAYER_NAME = "CLIMB"


	--Input booleans
		heor.input={};
		heor.input.key={};
		heor.input.keyPrevious={};
		heor.input.keyPress={};
		heor.input.keyRelease={};

		heor.input.key.inputU=false;
		heor.input.key.inputD=false;
		heor.input.key.inputL=false;
		heor.input.key.inputR=false;
		heor.input.key.inputThrow=false;
		heor.input.key.inputSwing=false;
		heor.input.key.inputWhip=false;

		heor.input.keyPrevious.inputU=false;
		heor.input.keyPrevious.inputD=false;
		heor.input.keyPrevious.inputL=false;
		heor.input.keyPrevious.inputR=false;
		heor.input.keyPrevious.inputThrow=false;
		heor.input.keyPrevious.inputSwing=false;
		heor.input.keyPrevious.inputWhip=false;

		heor.input.keyPress.inputU=false;
		heor.input.keyPress.inputD=false;
		heor.input.keyPress.inputL=false;
		heor.input.keyPress.inputR=false;
		heor.input.keyPress.inputThrow=false;
		heor.input.keyPress.inputSwing=false;
		heor.input.keyPress.inputWhip=false;

		heor.input.keyRelease.inputU=false;
		heor.input.keyRelease.inputD=false;
		heor.input.keyRelease.inputL=false;
		heor.input.keyRelease.inputR=false;
		heor.input.keyRelease.inputThrow=false;
		heor.input.keyRelease.inputSwing=false;
		heor.input.keyRelease.inputWhip=false;

	--Standing on Moving Platforms
		heor.platformVelocityX=0;
		heor.platformVelocityY=0;

	--C++ Interfacing
		heor.CPPInterface=nil;
		heor.mySprite=nil;
		heor.CompSprite=nil;
				heor.CompPosition=nil;
		heor.CompCollision=nil;
		heor.EID=0;
		heor.depth=0;
		heor.parent=0;
		heor.currentMap = nil

	--Height Maps
		heor.HMAP_HORIZONTAL= 0;
		heor.HMAP_VERTICAL	= 1;

	--entity collision
		heor.entityCollision = {}
		heor.entityCollision.primaryCollision = {}
		heor.entityCollision.primaryCollision.box = nil
		heor.entityCollision.primaryCollision.ID = 20

		heor.entityCollision.grabCollision = {}
		heor.entityCollision.grabCollision.box = nil
		heor.entityCollision.grabCollision.ID = 21
		heor.entityCollision.grabCollision.timer={}
		heor.entityCollision.grabCollision.timer.max=30
		heor.entityCollision.grabCollision.timer.current=0

--Collision boxes and ID Numbers
	local result=0;
	--security hole here? user can just use '..' to go wherever they want
	result, heor.tileCollision = pcall(loadfile(commonPath .. "/tileCollisionSystem.lua", _ENV))

--Sound Effects
	heor.SoundJump=nil;

--HUD
	heor.HUD={};
	heor.HUD.text={};
	heor.HUD.text.health=nil;

function heor.Initialize()
	-----------------------
	--C++ Interface setup--
	-----------------------
	heor.depth				= heor.LEngineData.depth;
	heor.parent				= heor.LEngineData.parent;
	heor.CPPInterface = CPP.interface; 
	heor.EID					= heor.LEngineData.entityID;

	if(heor.CPPInterface == nil) then heor.LEngineData.interface:WriteError(heor.EID, "CPPInterface is NIL!") end

	local EID = heor.EID
	heor.CPPInterface:ListenForInput(EID, "up"	 );
	heor.CPPInterface:ListenForInput(EID, "down" );
	heor.CPPInterface:ListenForInput(EID, "left" );
	heor.CPPInterface:ListenForInput(EID, "right");
	heor.CPPInterface:ListenForInput(EID, "throw");
	heor.CPPInterface:ListenForInput(EID, "swing");
	heor.CPPInterface:ListenForInput(EID, "whip" );

	heor.SoundJump = "smw_jump.wav";

	heor.coinText=heor.CPPInterface:RenderObjectText(EID, 400, 0, "Coins : " .. tostring(heor.coinCount));
	heor.coinText:SetColor(200,200,100);
	heor.coinText:SetDepth(-1004);

	heor.HUD.text.health=heor.CPPInterface:RenderObjectText(EID, 190, 0, "Health : " .. tostring(heor.health));
	heor.HUD.text.health:SetColor(200,20,10);
	heor.HUD.text.health:SetDepth(-1004);

	heor.CompCollision			= heor.CPPInterface:GetCollisionComponent(EID);
	heor.CompSprite		= heor.CPPInterface:GetSpriteComponent(EID);
	heor.CompPosition = heor.CPPInterface:GetPositionComponent(EID);

	heor.CompCollision:SetName(heor.LEngineData.name)
	heor.CompCollision:SetType(heor.LEngineData.objType);

	----------------
	--Sprite setup--
	----------------
	heor.mySprite			 = heor.CPPInterface:LoadSprite("SpriteLouie.xml");
	heor.mySpriteRoll  = heor.CPPInterface:LoadSprite("SpriteLouieRoll.xml");

	if( heor.mySprite==nil ) then
		heor.CPPInterface:WriteError("sprite is NIL");
	end

	--Logical origin is as at the top left; (0,0) is top left
	--Renderable origin is at center;				(-width/2, -width/2) is top left
	--To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
	heor.mySpriteID		= heor.CompSprite:AddSprite(heor.mySprite,	heor.depth, (heor.c.WIDTH/2), (heor.c.HEIGHT/2)+1);
	heor.mySpriteIDRoll = heor.CompSprite:AddSprite(heor.mySpriteRoll,	heor.depth, (heor.c.WIDTH/2), (heor.c.HEIGHT/2)+8);

	heor.CompSprite:SetAnimation			(heor.mySpriteID, "Stand");
	heor.CompSprite:SetAnimationSpeed (heor.mySpriteID, 1);
	heor.CompSprite:SetRotation				(heor.mySpriteID, 0);

	heor.CompSprite:SetAnimation			(heor.mySpriteIDRoll, "Roll");
	heor.CompSprite:SetAnimationSpeed (heor.mySpriteIDRoll, 1);
	heor.CompSprite:SetRotation				(heor.mySpriteIDRoll, 0);
	heor.CompSprite:RenderSprite			(heor.mySpriteIDRoll, false);

	-----------------------
	--Collision for tiles--
	-----------------------
	heor.tileCollision.Init(heor.c.WIDTH, heor.c.HEIGHT, heor.CPPInterface, heor.CompCollision, heor.EID);
	heor.tileCollision.callbackFunctions.TileUp		 = heor.OnTileUp;
	heor.tileCollision.callbackFunctions.TileDown  = heor.OnTileDown;
	heor.tileCollision.callbackFunctions.TileLeft  = heor.OnTileLeft;
	heor.tileCollision.callbackFunctions.TileRight = heor.OnTileRight

	--Primary collision
	heor.entityCollision.primaryCollision.box = CPP.CRect(0, 0, heor.c.WIDTH, heor.c.HEIGHT)
	heor.CompCollision:AddCollisionBox(heor.entityCollision.primaryCollision.box, heor.entityCollision.primaryCollision.ID, 30)
	heor.CompCollision:CheckForEntities(heor.entityCollision.primaryCollision.ID)
	heor.CompCollision:SetPrimaryCollisionBox(heor.entityCollision.primaryCollision.ID)
	--Grab collision
	heor.entityCollision.grabCollision.box = CPP.CRect(heor.c.WIDTH/2, heor.c.HEIGHT/2, heor.c.WIDTH/2,	1)
	heor.CompCollision:AddCollisionBox(heor.entityCollision.grabCollision.box, heor.entityCollision.grabCollision.ID, 30)
	heor.CompCollision:CheckForEntities(heor.entityCollision.grabCollision.ID)

	local position = heor.CompPosition:GetPositionWorld();

	heor.currentMap = heor.CPPInterface:GetMap()
	heor.climb.LAYER = heor.currentMap:GetTileLayer(heor.climb.LAYER_NAME)
end


function AngleToSignedAngle(a)
	if(a>180)then
		a= a - 360;
	end
	return a;
end

function SignedAngleToAngle(a)
	if(a<0)then
		a= a + 360;
	end
	return a;
end

function InvertHeightMap(hm)
	return 16-hm;
end

function heor.OnKeyDown(keyname)
	if(keyname=="up") then
		heor.input.key.inputU=true;
	elseif(keyname=="down") then
		heor.input.key.inputD=true;
	elseif(keyname=="left") then
		heor.input.key.inputL=true;
	elseif(keyname=="right") then
		heor.input.key.inputR=true;
	elseif(keyname=="throw") then
		heor.input.key.inputThrow=true;
	elseif(keyname=="swing") then
		heor.input.key.inputSwing=true;
	elseif(keyname=="whip") then
		heor.input.key.inputWhip=true;
	end
end

function heor.OnKeyUp(keyname)
	if(keyname=="up") then
		heor.input.key.inputU=false;
	elseif(keyname=="down") then
		heor.input.key.inputD=false;
	elseif(keyname=="left") then
		heor.input.key.inputL=false;
	elseif(keyname=="right") then
		heor.input.key.inputR=false;
	elseif(keyname=="throw") then
		heor.input.key.inputThrow=false;
	elseif(keyname=="swing") then
		heor.input.key.inputSwing=false;
	elseif(keyname=="whip") then
		heor.input.key.inputWhip=false;
	end
end

function heor.LockInput(frames)
	heor.lockTimer=frames;
	heor.inputLock=true;
end

function heor.UnlockInput()
	heor.inputLock=false;
	heor.lockTimer=0;
	heor.currentState=heor.c.STATE_NORMAL;
end

function heor.UpdateLock()
	if(heor.lockTimer>0)then
		heor.lockTimer= heor.lockTimer - 1;
	end
	if(heor.lockTimer==0)then
		heor.UnlockInput();
		heor.lockTimer=-1;
	end
	if(heor.roll_timer>0)then
		heor.roll_timer = heor.roll_timer - 1;
	end
end

function heor.LandOnPlatform()
	if(heor.tileCollision.groundTouch)then
		if(not heor.tileCollision.prevGroundTouch) then
			if( math.abs(heor.angleSigned)<25 )then
				if(heor.angleSigned>=0)then
					heor.groundSpeed = heor.xspd;
				else
					heor.groundSpeed = heor.xspd;
				end
			elseif( math.abs(heor.angleSigned)<=45 ) then
				if(math.abs(heor.xspd) > math.abs(heor.yspd))then
					heor.groundSpeed = heor.xspd;
				elseif(heor.angleSigned>0)then
					heor.groundSpeed = heor.yspd*-1*0.5*math.abs((math.cos(math.rad(heor.angle))));
				else
					heor.groundSpeed=heor.yspd*0.5*math.abs((math.cos(math.rad(heor.angle))));
				end

			else
				if(heor.angleSigned>0)then
					heor.groundSpeed = heor.yspd*-1--*math.abs((math.sin(math.rad(angle))));
				else
					heor.groundSpeed = heor.yspd--*math.abs((math.sin(math.rad(angle))));
				end
			end
			heor.yspd=0;
		end
	end
end

function heor.NewRopeSwing(maxTime, distanceFromCenter, direction)
	local CalculateY= function (x, dis) return ((x^2)/(4*dis))*-1; end
	local time=0;
	local moveDirection= direction or heor.facingDir;

	local maxDistance=distanceFromCenter;
	local x=-distanceFromCenter*moveDirection;
	local y=CalculateY(x,distanceFromCenter);
	local xPrevious=x;
	local yPrevious=y;

	return function()
		xPrevious=x;
		yPrevious=y;

		if(math.abs(x)>maxDistance)then return nil; end

		x= x + ((maxDistance/maxTime)*moveDirection);
		y= CalculateY(x, distanceFromCenter);

		heor.CompSprite:SetRotation(heor.mySpriteID, -75*moveDirection);
		--CPPInterface:WriteError(tostring(math.sqrt( (x^2) * (y^2) ) / distanceFromCenter));

		--return movement vector
		return CPP.Coord2df(x-xPrevious, y-yPrevious);
	end
end

function heor.ApplySlopeFactor()
	if(  ( (heor.angle>30)and(heor.angle<180) ) or ( (heor.angle<330)and(heor.angle>180) )	)then
		local extraSpeed = heor.c.SLOPE_GRAVITY * math.sin(math.rad(heor.angle))*-1;

		local zeroGSPD=(heor.groundSpeed==0);
		local positiveGSPD=(heor.groundSpeed>0);
		local newGSPD;
		heor.groundSpeed = heor.groundSpeed + extraSpeed;
		newGSPD=(heor.groundSpeed>0);
		if( (positiveGSPD~=newGSPD) and (not zeroGSPD) )then
			--lock input if direction changes
			--LockInput(30);
		end
	end
end

function heor.ApplyFrictionGravity()
	local friction; --Friction value used for this frame
	local gravityFrame; --Gravity for this frame;
	if(heor.currentState==heor.c.STATE_CLIMB)then
		return
	end
	if(heor.currentState==heor.c.STATE_WALLSLIDE)then
		gravityFrame=heor.c.WALLJUMP_GRAVITY;
	else
		gravityFrame=heor.c.GRAVITY;
	end
	friction= heor.c.FRICTION_MODIFER * heor.tileFriction

	--GRAVITY
	if (not heor.tileCollision.groundTouch) then
		heor.yspd = heor.yspd+gravityFrame;
		heor.CompSprite:SetRotation(heor.mySpriteID, 0);
		heor.angle=0;
		if(heor.currentState==heor.c.STATE_WALLSLIDE)then
			if(heor.yspd > heor.c.WALLJUMP_YSPD_MAX)then
				heor.yspd = heor.c.WALLJUMP_YSPD_MAX;
			end
		end
	end

	if ((not heor.input.key.inputL) and (not heor.input.key.inputR) and (heor.tileCollision.groundTouch))then
		friction=friction*5;
	end
	if ( ((not heor.input.key.inputL) and (not heor.input.key.inputR)) or (heor.inputLock) )then
		--IF TOUCHING THE GROUND
		if(heor.tileCollision.groundTouch)then
			if(heor.groundSpeed>0)then
				if(friction>=heor.groundSpeed)then
					heor.groundSpeed=0;
				else
					heor.groundSpeed = heor.groundSpeed - friction;
				end

			elseif(heor.groundSpeed<0)then
				if(heor.groundSpeed>=(friction*-1) )then
					heor.groundSpeed=0;
				else
					heor.groundSpeed = heor.groundSpeed + friction;
				end
			end
		--IF NOT TOUCHING THE GROUND
		else
			if(heor.xspd>0)then
				if(heor.c.FRICTION_AIR>=heor.xspd)then
					heor.xspd = 0;
				else
					heor.xspd = heor.xspd - heor.c.FRICTION_AIR;
				end

			elseif(heor.xspd < 0)then
				if(heor.xspd >= (heor.c.FRICTION_AIR*-1))then
					heor.xspd = 0;
				else
					heor.xspd= heor.xspd + heor.c.FRICTION_AIR;
				end
			end
		end
	end

end

function heor.MovementUpdatePipeline()
	heor.LandOnPlatform()
	heor.ApplySlopeFactor()
	heor.ApplyFrictionGravity()
end

function heor.InputJump()
		if heor.tileCollision.tileCollision.previous.tileUp == true then return; end
		heor.CPPInterface:PlaySound(heor.SoundJump);
		if(math.abs(heor.angleSigned)<25)then
			heor.yspd = heor.c.JUMPHEIGHT;
		else
			local yCos=math.abs(math.cos(math.rad(heor.angle)));
			local xSin=math.sin(math.rad(heor.angle));
			heor.yspd = (heor.c.JUMPHEIGHT * yCos) + ((heor.groundSpeed/4) * yCos)
			heor.xspd = (heor.c.JUMPHEIGHT * xSin) - (math.abs(heor.groundSpeed) * xSin)
		end

		heor.groundSpeed=0;
		heor.angle=0;
		heor.tileCollision.groundTouch=false;
		if(heor.currentState==heor.c.STATE_ROLL)then --long jump
			heor.xspd = heor.xspd * 1.5;
		end

		heor.CPPInterface:EventLuaBroadcastEvent(heor.EID, "JUMP");
		heor.UnlockInput();
end
function heor.InputWallJump()
		heor.CPPInterface:PlaySound(heor.SoundJump);
		heor.xspd = heor.c.WALLJUMP_XSPD * heor.facingDir;
		heor.yspd = -heor.c.WALLJUMP_YSPD;

		heor.ChangeState(heor.c.STATE_NORMAL);
		heor.LockInput(heor.c.WALLJUMP_LOCK);
end

function heor.InputHorizontal(direction)
	local absGS=math.abs(heor.groundSpeed);
	local absX= math.abs(heor.xspd);
	local movDir= (heor.groundSpeed>=0);
	if(movDir==false)then
		movDir=-1;
	else
		movDir=1;
	end

	--UPDATE GSPD (GROUND)
	if(heor.tileCollision.groundTouch) then
		if(movDir==direction) then --Add friction to the ground speed (slowing him faster) if moving against momentum
			if(absGS<heor.c.ACCELERATION_TOP)then
				heor.groundSpeed= heor.groundSpeed + (heor.c.ACCELERATION * direction);
			end
		else
			heor.groundSpeed= heor.groundSpeed + (heor.c.DEACCELERATION * direction);
		end
	end

	--UPDATE XSPD (AIR)
	if(not heor.tileCollision.groundTouch) then
		if(heor.currentState == heor.c.STATE_CLIMB)then
			heor.xspd = direction * heor.climb.SPEED
			heor.facingDir = direction
		else
			movDir= (heor.xspd>=0);
			if(movDir==false)then
			movDir=-1;
			else
				movDir=1;
			end
			if(movDir==direction) then
				if(absX<heor.c.ACCELERATION_TOP)then
				heor.xspd= heor.xspd + (heor.c.ACCELERATION_AIR * direction);
				end
			else
					 heor.xspd= heor.xspd + ((heor.c.ACCELERATION_AIR + heor.c.FRICTION_AIR) * direction);
			end
		end
	end
end
function heor.AnimateNormal()
	heor.CompSprite:RenderSprite			(heor.mySpriteIDRoll, false);
	heor.CompSprite:RenderSprite			(heor.mySpriteID, true);
	local newImgSpd=math.abs(heor.groundSpeed)/16;
	if(newImgSpd>2)then
		newImgSpd=2;
	end

	if(heor.groundSpeed > heor.c.ACCELERATION)then
		heor.CompSprite:SetAnimation		 (heor.mySpriteID, "Walk");
		heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, newImgSpd);
		heor.facingDir = heor.c.FACING_RIGHT;

	elseif(heor.groundSpeed < -(heor.c.ACCELERATION))then
		heor.CompSprite:SetAnimation		 (heor.mySpriteID, "Walk");
		heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, newImgSpd);
		heor.facingDir = heor.c.FACING_LEFT;

	elseif(heor.groundSpeed==0)then
		heor.CompSprite:SetAnimation		 (heor.mySpriteID,"Stand");
		heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, 0);
	end

	heor.CompSprite:SetScalingX(heor.mySpriteID, heor.facingDir);

	heor.CompSprite:SetScalingY(heor.mySpriteID, 1);


	if(heor.tileCollision.groundTouch)then
		heor.CompSprite:SetRotation(heor.mySpriteID, -heor.angle);
		--CompSprite:SetRotation(mySpriteID, -angle-rotationMode.mAngle);
	else
		heor.CompSprite:SetAnimation			(heor.mySpriteID, "Jump");
		heor.CompSprite:SetRotation(heor.mySpriteID, 0);
	end

end

function heor.AnimateOther()
	heor.CompSprite:RenderSprite			(heor.mySpriteIDRoll, false);
	heor.CompSprite:RenderSprite			(heor.mySpriteID, true);

	heor.CompSprite:SetAnimation		 (heor.mySpriteID, "Stand");
	heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, 0);

	heor.CompSprite:SetScaling			 (heor.mySpriteID, heor.facingDir, 1);

	if(heor.tileCollision.groundTouch)then
		heor.CompSprite:SetRotation(heor.mySpriteID, -heor.angle);
	else
		heor.CompSprite:SetRotation(heor.mySpriteID, 0);
	end

	if(heor.currentState == heor.c.STATE_WALLSLIDE)then
		heor.CompSprite:SetAnimation			(heor.mySpriteID, "WallSlide");
	end
end

function heor.AnimateRoll()
	heor.CompSprite:SetScalingY(heor.mySpriteIDRoll, 1);
	heor.CompSprite:SetRotation(heor.mySpriteIDRoll, (360*(heor.lockTimer/heor.c.ROLL_TIMER)) * -1 * heor.facingDir);
	heor.CompSprite:SetAnimation			(heor.mySpriteIDRoll, "Roll");

	heor.CompSprite:RenderSprite			(heor.mySpriteIDRoll, true);
	heor.CompSprite:RenderSprite			(heor.mySpriteID, false);
end

function heor.AnimateClimb()
	heor.CompSprite:SetScalingY(heor.mySpriteID, 1);
	heor.CompSprite:SetScalingX(heor.mySpriteID, 1);
	heor.CompSprite:SetRotation(heor.mySpriteID, 0)
	heor.CompSprite:SetAnimation(heor.mySpriteID, "Climb");
	if(heor.xspd == 0 and heor.yspd == 0)then
		heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, 0)
	else
		heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, .1)
	end

	heor.CompSprite:RenderSprite			(heor.mySpriteIDRoll, false);
	heor.CompSprite:RenderSprite			(heor.mySpriteID, true);
end

function heor.AnimateGrab()

end

function heor.UpdateInputs()
	for k,v in pairs(heor.input.key) do
		if			((v==true)and(heor.input.keyPrevious[k]==false))then
			heor.input.keyPress[k]=true;
			heor.input.keyRelease[k]=false;
		elseif	((v==false)and(heor.input.keyPrevious[k]==true))then
			heor.input.keyPress[k]=false;
			heor.input.keyRelease[k]=true;
		else
			heor.input.keyPress[k]=false;
			heor.input.keyRelease[k]=false;
		end
		heor.input.keyPrevious[k]=v;
	end
end

function heor.GrabItem()
	--activate collision box for specified time and play animation while time is going
	--Change width of colbox based on function of time
	--if item is grabbed, exit state to state holding
end

function heor.ChangeState(newState)
	heor.externalUseWhipInterrupt=true;
	if(newState == heor.c.STATE_NORMAL) then
		heor.UnlockInput();

	elseif(newState == heor.c.STATE_ROLL) then
		heor.LockInput(heor.c.ROLL_TIMER);

	elseif(newState==heor.c.STATE_WALLSLIDE) then
		heor.xspd=0;
		if(heor.yspd<0)then
			heor.yspd=0;
		end
	elseif(newState==heor.c.STATE_CLIMB) then
		heor.xspd=0
		heor.yspd=0
		heor.tileCollision.groundTouch=false
	end
	if (newState==nil)then
		assert(nil, "NEWSTATE IS NIL")

		heor.CPPInterface:WriteError(heor.EID, "NIL");
	end

	heor.currentState=newState;
end

function heor.CanClimb()
	local world = heor.CompPosition:GetPositionWorldInt()

	return heor.climb.LAYER:HasTile(world.x + (heor.c.WIDTH/2), world.y + (heor.c.HEIGHT/2))
end

function heor.Update()
	heor.externalUseWhipInterrupt=false;
	heor.externalUseWhip=false;
	if(heor.ropeSwing~=nil)then		 --if swinging, then just execute this block of code and return
		local returnVal=heor.ropeSwing();
		heor.platformVelocityX=0;
		heor.platformVelocityY=0;
		heor.CompPosition:ChangeParent(0);
		if(returnVal~=nil)then
			heor.CompPosition:SetMovement(returnVal);
			heor.xspd=returnVal.x;
			heor.yspd=-3.5;
			return;
		else
			heor.ropeSwing=nil;
		end
	end

	if(heor.currentState == heor.c.STATE_CLIMB)then
		if(heor.CanClimb() == false)then
			heor.ChangeState(heor.c.STATE_NORMAL)
		end
	end


	if((heor.tileCollision.groundTouch==true)and(heor.currentState == heor.c.STATE_WALLSLIDE))then
		heor.ChangeState(heor.c.STATE_NORMAL);
	end

	--if you land on the ground, turn off the attack lock
	if((heor.tileCollision.groundTouch==true)and(heor.attackLock==true))then
		heor.UnlockInput();
		heor.attackLock=false;
	end


	heor.UpdateInputs();
	heor.MovementUpdatePipeline();
	Vec2d = heor.CompPosition:GetPositionWorld();
	heor.angleSigned = AngleToSignedAngle(heor.angle);


	----------------
	--Handle Input--
	----------------
	if((heor.input.keyPress.inputSwing) and (not heor.inputLock))then
		heor.ropeSwing=heor.NewRopeSwing(18, 75); --these parameters are perfect :D
		heor.xspd=0;
		heor.yspd=0;
		heor.groundSpeed=0;
		heor.platformVelocityX=0;
		heor.platformVelocityY=0;
		heor.CompPosition:ChangeParent(0);
	end

	if((heor.input.keyPress.inputWhip) and (not heor.inputLock))then
		heor.externalUseWhip=true;
	end

	if((heor.input.key.inputThrow) and (not heor.inputLock))then
		if(heor.groundSpeed<0)then
			heor.groundSpeed = -heor.c.MAXSPEED;
		else
			heor.groundSpeed = heor.c.MAXSPEED;
		end
	end

	if (heor.input.keyPress.inputU) then
		if(heor.currentState == heor.c.STATE_WALLSLIDE)then
			heor.InputWallJump();
		elseif(heor.currentState == heor.c.STATE_CLIMB)then
			heor.yspd = -heor.climb.SPEED;
		elseif(heor.CanClimb())then
			heor.ChangeState(heor.c.STATE_CLIMB)
			heor.yspd = -heor.climb.SPEED

		elseif (heor.tileCollision.groundTouch) then
			heor.InputJump();
		end
	end
	if ( (heor.input.keyRelease.inputU) and (not heor.tileCollision.groundTouch) ) then
		if(heor.yspd<(-2))then
			heor.yspd=(-2);
		end
		if(heor.currentState == heor.c.STATE_CLIMB) then
			heor.yspd = 0
		end
	end
	if ( (heor.input.key.inputR) and (not heor.inputLock) ) then
		if((heor.tileCollision.tileCollision.previous.tileRight==true)and(not heor.tileCollision.groundTouch)and(heor.yspd>=0))then
			heor.facingDir = heor.c.FACING_LEFT; --face opposite way of wall slide
			heor.ChangeState(heor.c.STATE_WALLSLIDE);
		else
			heor.InputHorizontal(heor.c.FACING_RIGHT);
		end
	end
	if (heor.input.keyRelease.inputR) then
		if(heor.currentState == heor.c.STATE_WALLSLIDE)then
			if(heor.facingDir == heor.c.FACING_LEFT)then
				heor.ChangeState(heor.c.STATE_NORMAL);
			end
		end
		if(heor.currentState == heor.c.STATE_CLIMB) then
			heor.xspd = 0
		end
	end
	if ( (heor.input.key.inputL) and (not heor.inputLock) ) then
		if((heor.tileCollision.tileCollision.previous.tileLeft==true) and(not heor.tileCollision.groundTouch)and(heor.yspd>=0))then
			heor.facingDir = heor.c.FACING_RIGHT; --face opposite way of wall slide
			heor.ChangeState(heor.c.STATE_WALLSLIDE);
		else
			heor.InputHorizontal(heor.c.FACING_LEFT);
		end
	end
	if (heor.input.keyRelease.inputL) then
		if(heor.currentState==heor.c.STATE_WALLSLIDE)then
			if(heor.facingDir==heor.c.FACING_RIGHT)then
				heor.ChangeState(heor.c.STATE_NORMAL);
			end
		end
		if(heor.currentState == heor.c.STATE_CLIMB) then
			heor.xspd = 0
		end
	end

	if ( (heor.input.key.inputD) and (not heor.inputLock) ) then
		if((heor.currentState==heor.c.STATE_NORMAL)and(heor.tileCollision.groundTouch==true)and(heor.roll_timer==0))then
			heor.ChangeState(heor.c.STATE_ROLL);
			heor.roll_timer = heor.c.ROLL_COOLDOWN;
		elseif(heor.currentState==heor.c.STATE_CLIMB)then
			heor.yspd = heor.climb.SPEED
		end
	end

	if ( (heor.input.keyRelease.inputD) ) then
		if(heor.currentState == heor.c.STATE_CLIMB) then
			heor.yspd = 0
		end
	end

	-----------
	--Animate--
	-----------
	if		(heor.currentState == heor.c.STATE_ROLL)	then
		heor.AnimateRoll();

	elseif(heor.currentState == heor.c.STATE_NORMAL)then
		heor.AnimateNormal();

	elseif(heor.currentState == heor.c.STATE_CLIMB)then
		heor.AnimateClimb()

	else
		heor.AnimateOther();
		--heor.CPPInterface:WriteError(heor.EID, tostring(heor.currentState));
	end

	----------------------
	--Set X and Y Speeds--
	----------------------
	local speedDampenX=1;
	local speedDampenY=1;

	if(heor.currentState==heor.c.STATE_ROLL)then
		heor.groundSpeed = heor.c.ROLL_SPEED * heor.facingDir;
	end

	if(heor.groundSpeed > heor.c.MAXSPEED) then
		heor.groundSpeed = heor.c.MAXSPEED;
	elseif(heor.groundSpeed < (-heor.c.MAXSPEED)) then
		heor.groundSpeed=-heor.c.MAXSPEED;
	end

	if(heor.xspd > heor.c.MAXSPEED) then
		heor.xspd = heor.c.MAXSPEED;
	elseif(heor.xspd < (-heor.c.MAXSPEED)) then
		heor.xspd = -heor.c.MAXSPEED;
	end

	if(heor.yspd > heor.c.MAXSPEED) then
		heor.yspd = heor.c.MAXSPEED;
	elseif(heor.yspd < (-heor.c.MAXSPEED)) then
		heor.yspd = -heor.c.MAXSPEED;
	end

	if(heor.tileCollision.groundTouch) then
		heor.xspd = heor.groundSpeed;
		heor.yspd = 0;
		speedDampenX=math.cos(math.rad(heor.angle));
	else
		speedDampenX=1;
		speedDampenY=1;
	end

	if(heor.tileCollision.groundTouch==false)then
		heor.xspd = heor.xspd --+ heor.platformVelocityX;
		heor.yspd = heor.yspd --+ heor.platformVelocityY;
		heor.CompPosition:ChangeParent(heor.parent);
		platformVelocityX=0;
		platformVelocityY=0;
	end

	-------------------
	--Update CPP Data--
	-------------------
	--
	updateVec=CPP.Coord2df((heor.xspd*speedDampenX), (heor.yspd*speedDampenY));

	heor.CompPosition:SetMovement(updateVec);

	-----------------------------------------
	--Prepare for TCollision and Next Frame--
	-----------------------------------------

	heor.UpdateLock();

	Vec2d = heor.CompPosition:GetPositionWorld();
	local xx=Vec2d.x;
	local yy=Vec2d.y;

	if(heor.currentState==heor.c.STATE_WALLSLIDE)then
		if((heor.tileCollision.tileCollision.previous.tileRight==false)and(heor.facingDir==heor.c.FACING_LEFT))
		or((heor.tileCollision.tileCollision.previous.tileLeft==false) and(heor.facingDir==heor.c.FACING_RIGHT))then
			heor.ChangeState(heor.c.STATE_NORMAL);
		end
	end

	heor.prevGroundSpeed = heor.groundSpeed
	heor.CollisionUpdate()
end

function heor.CollisionUpdate()
	heor.tileCollision.Update(heor.xspd, heor.yspd)
	if(heor.currentState == heor.c.STATE_GRAB)then
		heor.CompCollision:Activate(heor.entityCollision.grabCollision.ID);
		heor.CompCollision:ChangeWidth(heor.entityCollision.grabCollision.ID, heor.c.WIDTH/2 * heor.facingDir);
	else
		heor.CompCollision:Deactivate(heor.entityCollision.grabCollision.ID);
	end
end

function heor.LandOnGround(ycoordinate, angleGround)
	--Update position
		newPosition= CPP.Coord2df(0,ycoordinate);

		newPosition= heor.CompPosition:TranslateWorldToLocal(newPosition);

		heor.CompPosition:SetPositionLocalY(newPosition.y);

		--Update variables
		heor.tileCollision.groundTouch=true;
		heor.angle=angleGround;
end

function heor.OnEntityCollision(entityID, packet)
	--Need to get height and collision type
	local leeway=10;
	local myBoxID=packet:GetID();
	local objectType=packet:GetType();
	local objectName=packet:GetName();
	local otherEntity = heor.CPPInterface:EntityGetInterface(entityID)
	local bounceThreshold = 3;

	if ((myBoxID==heor.entityCollision.primaryCollision.ID))then
		local solid = otherEntity.IsSolid();
		local bounce = otherEntity.CanBounce();
		if(solid==true)then
			if(bounce==true)then
				if(heor.yspd >= bounceThreshold)then
					local thisPos = heor.CPPInterface:EntityGetPositionWorld(heor.EID)
					local otherPos = heor.CPPInterface:EntityGetPositionWorld(entityID)
					if((thisPos.y+heor.c.HEIGHT) <= (otherPos.y + leeway) )then
						heor.yspd = heor.yspd * -.9
						otherEntity.Attack(1);
					end
				end
			end
			if(heor.yspd>=0)then
				local thisPos = heor.CPPInterface:EntityGetPositionWorld(heor.EID)
				local otherPos = heor.CPPInterface:EntityGetPositionWorld(entityID)
				if((thisPos.y+heor.c.HEIGHT) <= (otherPos.y + leeway) )then
					heor.tileCollision.groundTouch=true;
					heor.LandOnGround(otherPos.y-heor.c.HEIGHT, 0);
					heor.CompPosition:ChangeParent(entityID);

					local vecMove = heor.CPPInterface:EntityGetMovement(entityID);
					heor.platformVelocityX=vecMove.x;
					heor.platformVelocityY=vecMove.y;

					heor.angle=0;

					otherEntity.Land();
				end
			end
		end
	end
end


function heor.OnTileCollision(packet)
	local absoluteCoords=heor.CompPosition:GetPositionWorldInt();
	heor.tileCollision.OnTileCollision(packet, heor.xspd+heor.platformVelocityX, heor.yspd+heor.platformVelocityY, absoluteCoords.x, absoluteCoords.y);
end

function heor.OnTileDown(newPosition, newAngle)
	--Update position
	local absoluteCoords=heor.CompPosition:GetPositionWorld();
	newPosition= heor.CompPosition:TranslateWorldToLocal(newPosition);
	--newPosition.y=absoluteCoords.y-newPosition.y;
	--newPosition.x=0;
	heor.CompPosition:SetPositionLocalY(newPosition.y);

	--Update variables
	heor.angle=newAngle;
	heor.debug_tcolx=tx;
	heor.debug_tcoly=ty;
	heor.CompPosition:ChangeParent(heor.parent);
	heor.platformVelocityX=0;
	heor.platformVelocityY=0;
end

function heor.OnTileRight(newPosition)
	newPosition= heor.CompPosition:TranslateWorldToLocal(newPosition);
	heor.CompPosition:SetPositionLocalX(newPosition.x);
	heor.xspd=0; --for when in the air
	heor.groundSpeed=0;
end

function heor.OnTileLeft(newPosition)
	newPosition= heor.CompPosition:TranslateWorldToLocal(newPosition);
	heor.CompPosition:SetPositionLocalX(newPosition.x);
	heor.xspd=0; --for when in the air
	heor.groundSpeed=0;
end

function heor.OnTileUp(newPosition)
	if (heor.yspd<0) then
		heor.CompPosition:TranslateWorldToLocal(newPosition);
		heor.CompPosition:SetPositionLocalY(newPosition.y);
		heor.yspd=0;
	end
	heor.CompSprite:SetAnimationSpeed(heor.mySpriteID, 0);
end

function heor.TakeDamage(hitpoints)
	heor.xspd = heor.c.KNOCKBACK_SPEED_X * heor.facingDir*-1;
	heor.yspd = heor.c.KNOCKBACK_SPEED_Y;
	heor.attackLock=true;
	heor.LockInput(30);

	heor.health = heor.health-hitpoints;
	heor.HUD.text.health:ChangeText("Health : " .. tostring(heor.health));
end

function heor.Attacked(damage)
		if(heor.currentState ~= heor.c.STATE_ROLL)then--if not rolling
			heor.TakeDamage(damage);
			return true --was hit
		end
		return false; --not hit
end

function heor.OnLuaEvent(senderEID, eventString)

end

heor.EntityInterface = {
		IsSolid		= function ()				return true; end,
		GetHealth = function ()				return heor.health; end,
		Attack		= function (damage) return heor.Attacked(damage); end
	}

return heor;
end

return NewHeor;
