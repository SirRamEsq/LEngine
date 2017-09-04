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

local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewLouie(baseclass)
	function SetConstants(louie)
		--Constants
		louie.c = {}
		--Keys
			louie.c.K_UP = "up"
			louie.c.K_DOWN = "down"
			louie.c.K_LEFT = "left"
			louie.c.K_RIGHT = "right"

		louie.c.TILE_WIDTH=16;
		louie.c.TILE_HEIGHT=16;

		louie.c.MAXSPEED=16;
		louie.c.MAXHEALTH=10;

		louie.c.COL_WIDTH=18; --WIDTH of object sprite
		louie.c.COL_HEIGHT=32; --HEIGHT of object sprite
		louie.c.WIDTH=18; --WIDTH of object sprite
		louie.c.HEIGHT=32; --HEIGHT of object sprite

		louie.c.GRAVITY=0.21875;
		louie.c.SLOPE_GRAVITY=0.15
		louie.c.JUMPHEIGHT=-6;
		--slightly higher than a regular jump
		louie.c.JUMPHEIGHT_BOX = louie.c.JUMPHEIGHT - 1;
		louie.c.ACCELERATION=.046875*2;
		louie.c.ACCELERATION_AIR= louie.c.ACCELERATION;
		louie.c.DEACCELERATION=1;
		louie.c.ACCELERATION_TOP=4; --Max Speed Louie can acheive through normal acceleration
		louie.c.ROLL_SPEED= louie.c.ACCELERATION_TOP+1;
		louie.c.ROLL_TIMER=15;
		louie.c.ROLL_COOLDOWN=20;
		louie.roll_timer=0;

		louie.c.FRICTION_AIR=.1;
		louie.c.FRICTION_MODIFER=0.46875*2; --The friction of a tile is multiplied by this constant to get the actual friction value
		louie.c.MAX_STEPUP_PIXELS = 12;

		louie.c.WALLJUMP_LOCK=10;
		louie.c.WALLJUMP_XSPD=5;
		louie.c.WALLJUMP_YSPD=5;
		louie.c.WALLJUMP_GRAVITY=louie.c.GRAVITY/4;
		louie.c.WALLJUMP_YSPD_MAX=2;

		louie.c.STATE_NORMAL	= 0
		louie.c.STATE_ROLL		= 1
		louie.c.STATE_GRABLEDGE	= 2
		louie.c.STATE_WALLSLIDE	= 3
		louie.c.STATE_CLIMB		= 4
		louie.c.STATE_GRAB		= 5

		louie.c.FACING_LEFT=-1;
		louie.c.FACING_RIGHT=1;

		louie.c.KNOCKBACK_SPEED_X=2.5;
		louie.c.KNOCKBACK_SPEED_Y=-3;
	end

	local louie = baseclass or {}
	SetConstants(louie)

	function louie.InitVariables()
		--Useful functions
			local result
			local common
			result, common = pcall(loadfile(commonPath .. "/commonFunctions.lua", _ENV))
			louie.common = common

		--General Movement Variables
			louie.health= 2 
			louie.coinCount=0;
			louie.xspd=0;
			louie.yspd=0;
			louie.groundSpeed=0;
			louie.prevGroundSpeed=0; -- Ground speed of previous frame
			louie.tileFriction=0.046875;
			louie.inputLock=false;
			louie.lockTimer=0;

			louie.attackLock=false --character locked from being hit
			louie.angle=0;
			louie.angleSigned=0;
			louie.facingDir=louie.c.FACING_RIGHT;

			louie.currentState= louie.c.STATE_NORMAL;

		--CLIMBING
			louie.climb = {}
			louie.climb.SPEED = 2
			louie.climb.LAYER = nil
			louie.climb.LAYER_NAME = "CLIMB"

		--Input
			result, louie.input = pcall(loadfile(commonPath .. "/input.lua", _ENV))

		--Standing on Moving Platforms
			louie.platformVelocityX=0;
			louie.platformVelocityY=0;

		--C++ Interfacing
			louie.mainSprite = nil;
			louie.mainSpriteRoll = nil
			louie.baldSprite = nil
			louie.baldSpriteRoll = nil
			louie.currentSpriteID = 0
			louie.currentSpriteRollID = 0

			louie.CompSprite=nil;
			louie.CompPosition=nil;
			louie.CompCollision=nil;

			louie.EID=0;
			louie.depth=0;
			louie.parent=0;
			louie.currentMap = nil

		--Height Maps
			louie.HMAP_HORIZONTAL= 0;
			louie.HMAP_VERTICAL	= 1;

		--Entity collision
			louie.entityCollision = {}
			louie.entityCollision.primaryCollision = {}
			louie.entityCollision.primaryCollision.box = nil
			louie.entityCollision.primaryCollision.ID = 20

			louie.entityCollision.grabCollision = {}
			louie.entityCollision.grabCollision.box = nil
			louie.entityCollision.grabCollision.ID = 21
			louie.entityCollision.grabCollision.timer={}
			louie.entityCollision.grabCollision.timer.max=30
			louie.entityCollision.grabCollision.timer.current=0


		--Collision
			--security hole here? user can just use '..' to go wherever they want
			result, louie.tileCollision = pcall(loadfile(commonPath .. "/tileCollisionSystemNew.lua", _ENV))

		--Sound Effects
			louie.SoundJump = "smw_jump.wav";
	end

	function louie.Initialize()
		louie.InitVariables()

		-----------------------
		--C++ Interface setup--
		-----------------------
		louie.depth	= louie.LEngineData.depth;
		louie.parent= louie.LEngineData.parent;
		louie.EID	= louie.LEngineData.entityID;
		louie.name = louie.LEngineData.name
		louie.objType = louie.LEngineData.objType

		local EID = louie.EID
		CPP.interface:ListenForInput(EID, louie.c.K_UP);
		CPP.interface:ListenForInput(EID, louie.c.K_DOWN);
		CPP.interface:ListenForInput(EID, louie.c.K_LEFT);
		CPP.interface:ListenForInput(EID, louie.c.K_RIGHT);
		louie.input.RegisterKey( louie.c.K_UP   );
		louie.input.RegisterKey( louie.c.K_DOWN );
		louie.input.RegisterKey( louie.c.K_LEFT );
		louie.input.RegisterKey( louie.c.K_RIGHT);

		louie.CompCollision = CPP.interface:GetCollisionComponent(EID);
		louie.CompSprite	= CPP.interface:GetSpriteComponent(EID);
		louie.CompPosition  = CPP.interface:GetPositionComponent(EID);

		louie.CompCollision:SetName(louie.name)
		louie.CompCollision:SetType(louie.objType);

		----------------
		--Sprite setup--
		----------------
		louie.mainSprite	 = CPP.interface:LoadSprite("SpriteLouie.xml");
		louie.mainSpriteRoll = CPP.interface:LoadSprite("SpriteLouieRoll.xml");
		louie.baldSprite	 = CPP.interface:LoadSprite("SpriteLouieBald.xml");
		louie.baldSpriteRoll = CPP.interface:LoadSprite("SpriteLouieBaldRoll.xml");

		if( louie.mainSprite==nil ) then
			CPP.interface:WriteError(louie.EID, "sprite is NIL");
		end
		if( louie.baldSprite==nil ) then
			CPP.interface:WriteError(louie.EID, "bald sprite is NIL");
		end

		--Logical origin is as at the top left; (0,0) is top left
		--Renderable origin is at center;				(-width/2, -width/2) is top left
		--To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
		louie.mainSpriteID		= louie.CompSprite:AddSprite(louie.mainSprite,	louie.depth, (louie.c.COL_WIDTH/2), (louie.c.COL_HEIGHT/2)+1);
		louie.mainSpriteRollID = louie.CompSprite:AddSprite(louie.mainSpriteRoll,	louie.depth, (louie.c.COL_WIDTH/2), (louie.c.COL_HEIGHT/2)+8);
		louie.baldSpriteID		= louie.CompSprite:AddSprite(louie.baldSprite,	louie.depth, (louie.c.COL_WIDTH/2), (louie.c.COL_HEIGHT/2)+1);
		louie.baldSpriteRollID = louie.CompSprite:AddSprite(louie.baldSpriteRoll,	louie.depth, (louie.c.COL_WIDTH/2), (louie.c.COL_HEIGHT/2)+8);

		louie.CompSprite:SetAnimation		(louie.mainSpriteID, "Stand");
		louie.CompSprite:SetAnimationSpeed  (louie.mainSpriteID, 1);
		louie.CompSprite:SetRotation		(louie.mainSpriteID, 0);

		louie.CompSprite:SetAnimation		(louie.mainSpriteRollID, "Roll");
		louie.CompSprite:SetAnimationSpeed  (louie.mainSpriteRollID, 1);
		louie.CompSprite:SetRotation		(louie.mainSpriteRollID, 0);
		louie.CompSprite:RenderSprite		(louie.mainSpriteRollID, false);

		-----------------------
		--Collision for tiles--
		-----------------------
		louie.tileCollision.Init(louie.c.COL_WIDTH, louie.c.COL_HEIGHT, CPP.interface, louie.CompCollision, louie.EID);
		louie.tileCollision.callbackFunctions.TileUp	= louie.OnTileUp;
		louie.tileCollision.callbackFunctions.TileDown  = louie.OnTileDown;
		louie.tileCollision.callbackFunctions.TileLeft  = louie.OnTileLeft;
		louie.tileCollision.callbackFunctions.TileRight = louie.OnTileRight

		--Primary collision
		louie.entityCollision.primaryCollision.box = CPP.CRect(0, 0, louie.c.COL_WIDTH, louie.c.COL_HEIGHT)
		louie.CompCollision:AddCollisionBox(louie.entityCollision.primaryCollision.box, louie.entityCollision.primaryCollision.ID, 30)
		louie.CompCollision:CheckForEntities(louie.entityCollision.primaryCollision.ID)
		louie.CompCollision:SetPrimaryCollisionBox(louie.entityCollision.primaryCollision.ID)

		--[[Grab collision
		louie.entityCollision.grabCollision.box = CPP.CRect(louie.c.COL_WIDTH/2, louie.c.COL_HEIGHT/2, louie.c.COL_WIDTH/2,	1)
		louie.CompCollision:AddCollisionBox(louie.entityCollision.grabCollision.box, louie.entityCollision.grabCollision.ID, 30)
		louie.CompCollision:CheckForEntities(louie.entityCollision.grabCollision.ID)
		--]]

		louie.currentMap = CPP.interface:GetMap()
		louie.climb.LAYER = louie.currentMap:GetTileLayer(louie.climb.LAYER_NAME)
	end

	function louie.OnKeyDown(keyname)
		louie.input.OnKeyDown(keyname)
	end

	function louie.OnKeyUp(keyname)
		louie.input.OnKeyUp(keyname)
	end

	function louie.SetCollisionBoxes()
		if(louie.currentState == louie.c.STATE_ROLL)then
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_RIGHT_SHORT)
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_LEFT_SHORT)
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_RIGHT)
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_LEFT)
		else
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_RIGHT_SHORT)
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_LEFT_SHORT)
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_RIGHT)
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_LEFT)
		end
	end

	function louie.Update()
		louie.SetCollisionBoxes()
		louie.Climb()	
		louie.WallSlide()
		louie.UpdateInputs()

		louie.LandOnPlatform()
		louie.ApplySlopeFactor()
		louie.ApplyFrictionGravity()

		louie.HandleInput()
		louie.Animate()

		louie.UpdateCPP()
		louie.UpdateGUI()

		louie.PrepareForNextFrame()
	end


	function louie.AnimateNormal()
		louie.CompSprite:RenderSprite			(louie.currentSpriteRollID, false);
		louie.CompSprite:RenderSprite			(louie.currentSpriteID, true);
		local newImgSpd=math.abs(louie.groundSpeed)/16;
		if(newImgSpd>2)then
			newImgSpd=2;
		end

		if(louie.groundSpeed > louie.c.ACCELERATION)then
			louie.CompSprite:SetAnimation		 (louie.currentSpriteID, "Walk");
			louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, newImgSpd);
			louie.facingDir = louie.c.FACING_RIGHT;

		elseif(louie.groundSpeed < -(louie.c.ACCELERATION))then
			louie.CompSprite:SetAnimation		 (louie.currentSpriteID, "Walk");
			louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, newImgSpd);
			louie.facingDir = louie.c.FACING_LEFT;

		elseif(louie.groundSpeed==0)then
			louie.CompSprite:SetAnimation		 (louie.currentSpriteID,"Stand");
			louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, 0);
		end

		louie.CompSprite:SetScalingX(louie.currentSpriteID, louie.facingDir);

		louie.CompSprite:SetScalingY(louie.currentSpriteID, 1);


		if(louie.tileCollision.groundTouch)then
			louie.CompSprite:SetRotation(louie.currentSpriteID, -louie.angle);
			--CompSprite:SetRotation(currentSpriteID, -angle-rotationMode.mAngle);
		else
			louie.CompSprite:SetAnimation			(louie.currentSpriteID, "Jump");
			louie.CompSprite:SetRotation(louie.currentSpriteID, 0);
		end

	end

	function louie.AnimateOther()
		louie.CompSprite:RenderSprite			(louie.currentSpriteRollID, false);
		louie.CompSprite:RenderSprite			(louie.currentSpriteID, true);

		louie.CompSprite:SetAnimation		 (louie.currentSpriteID, "Stand");
		louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, 0);

		louie.CompSprite:SetScaling			 (louie.currentSpriteID, louie.facingDir, 1);

		if(louie.tileCollision.groundTouch)then
			louie.CompSprite:SetRotation(louie.currentSpriteID, -louie.angle);
		else
			louie.CompSprite:SetRotation(louie.currentSpriteID, 0);
		end

		if(louie.currentState == louie.c.STATE_WALLSLIDE)then
			louie.CompSprite:SetAnimation			(louie.currentSpriteID, "WallSlide");
		end
	end

	function louie.AnimateRoll()
		louie.CompSprite:SetScalingY(louie.currentSpriteRollID, 1);
		louie.CompSprite:SetRotation(louie.currentSpriteRollID, (360*(louie.lockTimer/louie.c.ROLL_TIMER)) * -1 * louie.facingDir);
		louie.CompSprite:SetAnimation			(louie.currentSpriteRollID, "Roll");

		louie.CompSprite:RenderSprite			(louie.currentSpriteRollID, true);
		louie.CompSprite:RenderSprite			(louie.currentSpriteID, false);
	end

	function louie.AnimateClimb()
		louie.CompSprite:SetScalingY(louie.currentSpriteID, 1);
		louie.CompSprite:SetScalingX(louie.currentSpriteID, 1);
		louie.CompSprite:SetRotation(louie.currentSpriteID, 0)
		louie.CompSprite:SetAnimation(louie.currentSpriteID, "Climb");
		if(louie.xspd == 0 and louie.yspd == 0)then
			louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, 0)
		else
			louie.CompSprite:SetAnimationSpeed(louie.currentSpriteID, .1)
		end

		louie.CompSprite:RenderSprite			(louie.currentSpriteRollID, false);
		louie.CompSprite:RenderSprite			(louie.currentSpriteID, true);
	end

	function louie.Animate()
		if (louie.health <= 1) then
			louie.currentSpriteID = louie.baldSpriteID
			louie.currentSpriteRollID = louie.baldSpriteRollID
			louie.CompSprite:RenderSprite(louie.mainSpriteID, false);
			louie.CompSprite:RenderSprite(louie.mainSpriteRollID, false);
		else
			louie.currentSpriteID = louie.mainSpriteID
			louie.currentSpriteRollID = louie.mainSpriteRollID
			louie.CompSprite:RenderSprite(louie.baldSpriteID, false);
			louie.CompSprite:RenderSprite(louie.baldSpriteRollID, false);
		end

		if	(louie.currentState == louie.c.STATE_ROLL)	then
			louie.AnimateRoll();

		elseif(louie.currentState == louie.c.STATE_NORMAL)then
			louie.AnimateNormal();

		elseif(louie.currentState == louie.c.STATE_CLIMB)then
			louie.AnimateClimb()

		else
			louie.AnimateOther();
		end

	end

	function louie.CanClimb()
		if(louie.climb.LAYER == nil)then return end
		local world = louie.CompPosition:GetPositionWorld():Round()

		return louie.climb.LAYER:HasTile( (world.x + (louie.c.WIDTH/2))/16, (world.y + (louie.c.HEIGHT/2))/16 )
	end

	function louie.Climb()
		if(louie.currentState == louie.c.STATE_CLIMB)then
			if(louie.CanClimb() == false)then
				louie.ChangeState(louie.c.STATE_NORMAL)
			end
		end
	end

	function louie.WallSlide()
		if((louie.tileCollision.groundTouch==true)and(louie.currentState == louie.c.STATE_WALLSLIDE))then
			louie.ChangeState(louie.c.STATE_NORMAL);
		end
	end


	function louie.UpdateInputs()
		louie.input.Update()
	end

	function louie.LandOnPlatform()
		if( (louie.tileCollision.groundTouch) and (not louie.tileCollision.prevGroundTouch) ) then
			if( math.abs(louie.angleSigned)<25 )then
				if(louie.angleSigned>=0)then
					louie.groundSpeed = louie.xspd;
				else
					louie.groundSpeed = louie.xspd;
				end
			elseif( math.abs(louie.angleSigned)<=45 ) then
				if(math.abs(louie.xspd) > math.abs(louie.yspd))then
					louie.groundSpeed = louie.xspd;
				elseif(louie.angleSigned>0)then
					louie.groundSpeed = louie.yspd*-1*0.5*math.abs((math.cos(math.rad(louie.angle))));
				else
					louie.groundSpeed=louie.yspd*0.5*math.abs((math.cos(math.rad(louie.angle))));
				end

			else
				if(louie.angleSigned>0)then
					louie.groundSpeed = louie.yspd*-1--*math.abs((math.sin(math.rad(angle))));
				else
					louie.groundSpeed = louie.yspd--*math.abs((math.sin(math.rad(angle))));
				end
			end
			louie.yspd=0;
		end
	end

	function louie.ApplySlopeFactor()
		if(  ( (louie.angle>30)and(louie.angle<180) ) or ( (louie.angle<330)and(louie.angle>180) )	)then
			local extraSpeed = louie.c.SLOPE_GRAVITY * math.sin(math.rad(louie.angle))*-1;

			local zeroGSPD=(louie.groundSpeed==0);
			local positiveGSPD=(louie.groundSpeed>0);
			local newGSPD;
			louie.groundSpeed = louie.groundSpeed + extraSpeed;
			newGSPD=(louie.groundSpeed>0);
			if( (positiveGSPD~=newGSPD) and (not zeroGSPD) )then
				--lock input if direction changes
				--LockInput(30);
			end
		end
	end

	function louie.ApplyFrictionGravity()
		local friction; --Friction value used for this frame
		local gravityFrame; --Gravity for this frame;
		if(louie.currentState==louie.c.STATE_CLIMB)then
			return
		end
		if(louie.currentState==louie.c.STATE_WALLSLIDE)then
			gravityFrame=louie.c.WALLJUMP_GRAVITY;
		else
			gravityFrame=louie.c.GRAVITY;
		end
		friction= louie.c.FRICTION_MODIFER * louie.tileFriction

		--GRAVITY
		if (not louie.tileCollision.groundTouch) then
			louie.yspd = louie.yspd+gravityFrame;
			louie.CompSprite:SetRotation(louie.mainSpriteID, 0);
			louie.angle=0;
			if(louie.currentState==louie.c.STATE_WALLSLIDE)then
				if(louie.yspd > louie.c.WALLJUMP_YSPD_MAX)then
					louie.yspd = louie.c.WALLJUMP_YSPD_MAX;
				end
			end
		end

		if ((not louie.input.key[louie.c.K_LEFT]) and (not louie.input.key[louie.c.K_RIGHT]) and (louie.tileCollision.groundTouch))then
			friction=friction*5;
		end
		if ( ((not louie.input.key[louie.c.K_LEFT]) and (not louie.input.key[louie.c.K_RIGHT])) or (louie.inputLock) )then
			--IF TOUCHING THE GROUND
			if(louie.tileCollision.groundTouch)then
				if(louie.groundSpeed>0)then
					if(friction>=louie.groundSpeed)then
						louie.groundSpeed=0;
					else
						louie.groundSpeed = louie.groundSpeed - friction;
					end

				elseif(louie.groundSpeed<0)then
					if(louie.groundSpeed>=(friction*-1) )then
						louie.groundSpeed=0;
					else
						louie.groundSpeed = louie.groundSpeed + friction;
					end
				end
			--IF NOT TOUCHING THE GROUND
			else
				if(louie.xspd>0)then
					if(louie.c.FRICTION_AIR>=louie.xspd)then
						louie.xspd = 0;
					else
						louie.xspd = louie.xspd - louie.c.FRICTION_AIR;
					end

				elseif(louie.xspd < 0)then
					if(louie.xspd >= (louie.c.FRICTION_AIR*-1))then
						louie.xspd = 0;
					else
						louie.xspd= louie.xspd + louie.c.FRICTION_AIR;
					end
				end
			end
		end
	end

	function louie.LockInput(frames)
		louie.lockTimer=frames;
		louie.inputLock=true;
	end

	function louie.UnlockInput()
		louie.inputLock=false;
		louie.lockTimer=0;
		louie.currentState=louie.c.STATE_NORMAL;
	end

	function louie.UpdateLock()
		if(louie.lockTimer>0)then
			louie.lockTimer= louie.lockTimer - 1;
		end
		if(louie.lockTimer==0)then
			louie.UnlockInput();
			louie.lockTimer=-1;
		end
		if(louie.roll_timer>0)then
			louie.roll_timer = louie.roll_timer - 1;
		end
	end

	function louie.InputJump()
		if louie.tileCollision.previous.tileUp == true then return; end
		CPP.interface:PlaySound(louie.SoundJump, 100);
		if(math.abs(louie.angleSigned)<25)then
			louie.yspd = louie.c.JUMPHEIGHT;
		else
			local yCos=math.abs(math.cos(math.rad(louie.angle)));
			local xSin=math.sin(math.rad(louie.angle));
			louie.yspd = (louie.c.JUMPHEIGHT * yCos) + ((louie.groundSpeed/4) * yCos)
			louie.xspd = (louie.c.JUMPHEIGHT * xSin) - (math.abs(louie.groundSpeed) * xSin)
		end

		louie.groundSpeed=0;
		louie.angle=0;
		louie.tileCollision.groundTouch=false;
		if(louie.currentState==louie.c.STATE_ROLL)then --long jump
			louie.xspd = louie.xspd * 1.5;
		end

		CPP.interface:EventLuaBroadcastEvent(louie.EID, "JUMP");
		louie.UnlockInput();
	end

	function louie.InputWallJump()
		CPP.interface:PlaySound(louie.SoundJump, 100);
		louie.xspd = louie.c.WALLJUMP_XSPD * louie.facingDir;
		louie.yspd = -louie.c.WALLJUMP_YSPD;

		louie.ChangeState(louie.c.STATE_NORMAL);
		louie.LockInput(louie.c.WALLJUMP_LOCK);
	end

	function louie.BoxJump()
		louie.yspd = louie.c.JUMPHEIGHT;
		if(not louie.input.key[louie.c.K_UP])then
			louie.yspd = -3
		end

		louie.groundSpeed=0;
		louie.angle=0;
		louie.tileCollision.groundTouch=false;

		--Up Key not Pressed
		if (not louie.input.key[louie.c.K_UP]) then
			louie.JumpCancel()
		end

		if(louie.currentState==louie.c.STATE_ROLL)then --long jump
			louie.xspd = louie.xspd * 1.5;
		end

		CPP.interface:EventLuaBroadcastEvent(louie.EID, "JUMP");
		louie.UnlockInput();
	end

	function louie.ChangeState(newState)
		louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_RIGHT_SHORT)
		louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_LEFT_SHORT)
		louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_RIGHT)
		louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_LEFT)
		if(newState == louie.c.STATE_NORMAL) then
			louie.UnlockInput();

		elseif(newState == louie.c.STATE_ROLL) then
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_RIGHT_SHORT)
			louie.CompCollision:Activate(louie.tileCollision.boxID.TILE_LEFT_SHORT)
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_RIGHT)
			louie.CompCollision:Deactivate(louie.tileCollision.boxID.TILE_LEFT)
			louie.LockInput(louie.c.ROLL_TIMER);

		elseif(newState==louie.c.STATE_WALLSLIDE) then
			louie.xspd=0;
			if(louie.yspd<0)then
				louie.yspd=0;
			end
		elseif(newState==louie.c.STATE_CLIMB) then
			louie.xspd=0
			louie.yspd=0
			louie.tileCollision.groundTouch=false
		end
		if (newState==nil)then
			assert(nil, "NEWSTATE IS NIL")

			CPP.interface:WriteError(louie.EID, "NIL");
		end

		louie.currentState = newState;
	end
	
	function louie.InputHorizontal(direction)
		local absGS=math.abs(louie.groundSpeed);
		local absX= math.abs(louie.xspd);
		local movDir= (louie.groundSpeed>=0);
		if(movDir==false)then
			movDir=-1;
		else
			movDir=1;
		end

		--UPDATE GSPD (GROUND)
		if(louie.tileCollision.groundTouch) then
			if(movDir==direction) then --Add friction to the ground speed (slowing him faster) if moving against momentum
				if(absGS<louie.c.ACCELERATION_TOP)then
					louie.groundSpeed= louie.groundSpeed + (louie.c.ACCELERATION * direction);
				end
			else
				louie.groundSpeed= louie.groundSpeed + (louie.c.DEACCELERATION * direction);
			end
		end

		--UPDATE XSPD (AIR)
		if(not louie.tileCollision.groundTouch) then
			if(louie.currentState == louie.c.STATE_CLIMB)then
				louie.xspd = direction * louie.climb.SPEED
				louie.facingDir = direction
			else
				movDir= (louie.xspd>=0);
				if(movDir==false)then
				movDir=-1;
				else
					movDir=1;
				end
				if(movDir==direction) then
					if(absX<louie.c.ACCELERATION_TOP)then
					louie.xspd= louie.xspd + (louie.c.ACCELERATION_AIR * direction);
					end
				else
						 louie.xspd= louie.xspd + ((louie.c.ACCELERATION_AIR + louie.c.FRICTION_AIR) * direction);
				end
			end
		end
	end

	function louie.JumpCancel()
		if(louie.yspd<(-2))then
			louie.yspd=(-2);
		end
	end

	function louie.HandleInput()
		--Up Pressed
		if (louie.input.keyPress[louie.c.K_UP]) then
			if(louie.currentState == louie.c.STATE_WALLSLIDE)then
				louie.InputWallJump();

			elseif(louie.currentState == louie.c.STATE_CLIMB)then
				louie.yspd = -louie.climb.SPEED;

			elseif(louie.CanClimb())then
				louie.ChangeState(louie.c.STATE_CLIMB)
				louie.yspd = -louie.climb.SPEED

			elseif (louie.tileCollision.groundTouch) then
				louie.InputJump();
			end
		end
		--Up Released
		if ( (louie.input.keyRelease[louie.c.K_UP]) and (not louie.tileCollision.groundTouch) ) then
			louie.JumpCancel()

			if(louie.currentState == louie.c.STATE_CLIMB) then
				louie.yspd = 0
			end
		end

		----------------------------------------------------------------------
		
		--Right Pressed
		if ( (louie.input.key[louie.c.K_RIGHT]) and (not louie.inputLock) ) then
			if((louie.tileCollision.previous.tileRight==true)and(not louie.tileCollision.groundTouch)and(louie.yspd>=0))then
				louie.facingDir = louie.c.FACING_LEFT; --face opposite way of wall slide
				louie.ChangeState(louie.c.STATE_WALLSLIDE);
			else
				louie.InputHorizontal(louie.c.FACING_RIGHT);
			end
		end
		--Right Released
		if (louie.input.keyRelease[louie.c.K_RIGHT]) then
			if(louie.currentState == louie.c.STATE_WALLSLIDE)then
				if(louie.facingDir == louie.c.FACING_LEFT)then
					louie.ChangeState(louie.c.STATE_NORMAL);
				end
			end
			if(louie.currentState == louie.c.STATE_CLIMB) then
				louie.xspd = 0
			end
		end

		----------------------------------------------------------------------

		--Left Pressed
		if ( (louie.input.key[louie.c.K_LEFT]) and (not louie.inputLock) ) then
			if((louie.tileCollision.previous.tileLeft==true) and(not louie.tileCollision.groundTouch)and(louie.yspd>=0))then
				louie.facingDir = louie.c.FACING_RIGHT; --face opposite way of wall slide
				louie.ChangeState(louie.c.STATE_WALLSLIDE);
			else
				louie.InputHorizontal(louie.c.FACING_LEFT);
			end
		end
		--Left Released
		if (louie.input.keyRelease[louie.c.K_LEFT]) then
			if(louie.currentState==louie.c.STATE_WALLSLIDE)then
				if(louie.facingDir==louie.c.FACING_RIGHT)then
					louie.ChangeState(louie.c.STATE_NORMAL);
				end
			end
			if(louie.currentState == louie.c.STATE_CLIMB) then
				louie.xspd = 0
			end
		end

		----------------------------------------------------------------------

		--Down Pressed
		if ( (louie.input.key[louie.c.K_DOWN]) and (not louie.inputLock) ) then
			if((louie.currentState==louie.c.STATE_NORMAL)and(louie.tileCollision.groundTouch==true)and(louie.roll_timer==0))then
				louie.ChangeState(louie.c.STATE_ROLL);
				louie.roll_timer = louie.c.ROLL_COOLDOWN;
			elseif(louie.currentState==louie.c.STATE_CLIMB)then
				louie.yspd = louie.climb.SPEED
			end
		end
		--Down Released
		if ( (louie.input.keyRelease[louie.c.K_DOWN]) ) then
			if(louie.currentState == louie.c.STATE_CLIMB) then
				louie.yspd = 0
			end
		end
	end

	function louie.UpdateCPP()
		local speedDampenX=1;
		local speedDampenY=1;

		if(louie.currentState==louie.c.STATE_ROLL)then
			louie.groundSpeed = louie.c.ROLL_SPEED * louie.facingDir;
		end

		if(louie.groundSpeed > louie.c.MAXSPEED) then
			louie.groundSpeed = louie.c.MAXSPEED;
		elseif(louie.groundSpeed < (-louie.c.MAXSPEED)) then
			louie.groundSpeed=-louie.c.MAXSPEED;
		end

		if(louie.xspd > louie.c.MAXSPEED) then
			louie.xspd = louie.c.MAXSPEED;
		elseif(louie.xspd < (-louie.c.MAXSPEED)) then
			louie.xspd = -louie.c.MAXSPEED;
		end

		if(louie.yspd > louie.c.MAXSPEED) then
			louie.yspd = louie.c.MAXSPEED;
		elseif(louie.yspd < (-louie.c.MAXSPEED)) then
			louie.yspd = -louie.c.MAXSPEED;
		end

		if(louie.tileCollision.groundTouch) then
			louie.xspd = louie.groundSpeed;
			louie.yspd = 0;
			speedDampenX=math.cos(math.rad(louie.angle));
		else
			speedDampenX=1;
			speedDampenY=1;
		end

		if(louie.tileCollision.groundTouch==false)then
			louie.xspd = louie.xspd --+ louie.platformVelocityX;
			louie.yspd = louie.yspd --+ louie.platformVelocityY;
			louie.CompPosition:SetParent(louie.parent);
			platformVelocityX=0;
			platformVelocityY=0;
		end
		updateVec=CPP.Coord2df((louie.xspd*speedDampenX), (louie.yspd*speedDampenY));
		louie.CompPosition:SetMovement(updateVec);
	end

	function louie.UpdateGUI()
		local resolution = CPP.interface:GetResolution()
		local windowFlags = imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove + imGuiFlags.AlwaysAutoResize

		--no background
		CPP.ImGui.PushStyleColorWindowBG(CPP.Color4f(0,0,0, 0))
		CPP.ImGui.PushStyleColorText(CPP.Color4f(.9,.1,.1,1))
		louie.guiName = "louieGUI"

		CPP.ImGui.BeginFlags(louie.guiName, windowFlags)
		CPP.ImGui.Text(tostring(louie.health))
		local winSize = CPP.ImGui.GetWindowSize()
		CPP.ImGui.End()
		CPP.ImGui.PopStyleColor(2)

		--Center Window
		local guiPosition = CPP.Coord2df(( resolution.x/2) - (winSize.x/2), 0)
		CPP.ImGui.SetWindowPos(louie.guiName, guiPosition, 0)
	end

	function louie.PrepareForNextFrame()
		louie.UpdateLock();

		Vec2d = louie.CompPosition:GetPositionWorld();
		local xx=Vec2d.x;
		local yy=Vec2d.y;

		if(louie.currentState==louie.c.STATE_WALLSLIDE)then
			if((louie.tileCollision.previous.tileRight==false)and(louie.facingDir==louie.c.FACING_LEFT))
			or((louie.tileCollision.previous.tileLeft==false) and(louie.facingDir==louie.c.FACING_RIGHT))then
				louie.ChangeState(louie.c.STATE_NORMAL);
			end
		end

		louie.prevGroundSpeed = louie.groundSpeed
		louie.CollisionUpdate()
	end

	function louie.CollisionUpdate()
		louie.tileCollision.Update(louie.xspd, louie.yspd)
	end
	
	function louie.LandOnGround(ycoordinate, angleGround)
		--Update position
		newPosition= CPP.Coord2df(0,ycoordinate);

		newPosition= louie.CompPosition:TranslateWorldToLocal(newPosition);

		louie.CompPosition:SetPositionLocalY(newPosition.y);

		--Update variables
		louie.tileCollision.groundTouch=true;
		louie.angle=angleGround;
	end

	function louie.OnEntityCollision(entityID, packet)
		--Need to get height and collision type
		local leeway=10;
		local myBoxID=packet:GetID();
		local objectType=packet:GetType();
		local objectName=packet:GetName();
		local otherEntity = CPP.interface:EntityGetInterface(entityID)
		local bounceThreshold = 3;

		if ((myBoxID==louie.entityCollision.primaryCollision.ID))then
			local solid = otherEntity.IsSolid();
			local bounce = otherEntity.CanBounce();
			if(solid==true)then
				if(bounce==true)then
					if(louie.yspd >= bounceThreshold)then
						local thisPos = CPP.interface:EntityGetPositionWorld(louie.EID)
						local otherPos = CPP.interface:EntityGetPositionWorld(entityID)
						if((thisPos.y+louie.c.HEIGHT) <= (otherPos.y + leeway) )then
							louie.yspd = louie.yspd * -.9
							otherEntity.Attack(1);
						end
					end
				end
				if(louie.yspd>=0)then
					local thisPos = CPP.interface:EntityGetPositionWorld(louie.EID)
					local otherPos = CPP.interface:EntityGetPositionWorld(entityID)
					if((thisPos.y+louie.c.HEIGHT) <= (otherPos.y + leeway) )then
						louie.tileCollision.groundTouch=true;
						louie.LandOnGround(otherPos.y-louie.c.HEIGHT, 0);
						louie.CompPosition:SetParent(entityID);

						local vecMove = CPP.interface:EntityGetMovement(entityID);
						louie.platformVelocityX=vecMove.x;
						louie.platformVelocityY=vecMove.y;

						louie.angle=0;

						otherEntity.Land();
					end
				end
			end
		end
	end

	function louie.GetHat()
		if(louie.health <= 1)then
			louie.health = 2
		end
	end

	function louie.BreakBox(layer, tx, ty)
		--Returns true if box is broken
	
		tileID = layer:GetTile(tx,ty)
		local hm = layer:UsesHMaps();
		local isBox = layer:GetTileProperty(tileID, "isBox")
		local boxType1 = 0

		if(isBox == "true")then
			local isBoxHat = layer:GetTileProperty(tileID, "isBoxHat")

			if(isBoxHat == "true")then
				boxType1 = 1
				louie.GetHat()
			end

			--destroy box
			layer:SetTile(tx,ty, 0)
			layer:UpdateRenderArea(CPP.CRect(tx,ty, 0,0))

			--Display box break effect
			CPP.interface:EntityNew('Effects/boxBreak.lua',tx*16,ty*16, louie.depth, 0, "BOXBREAK", "",
									{boxType = boxType1})

			--Return true, indicating box was broken
			return true
		end

		--Return false, indicating box was NOT broken
		return false
	end

	function louie.OnTileCollision(packet)
		local absoluteCoords=louie.CompPosition:GetPositionWorld():Round()
		louie.tileCollision.OnTileCollision(packet, louie.xspd+louie.platformVelocityX, louie.yspd+louie.platformVelocityY, absoluteCoords.x, absoluteCoords.y);
	end

	function louie.OnTileDown(newPosition, newAngle, layer, tx, ty)
		--Update position
		local absoluteCoords=louie.CompPosition:GetPositionWorld();
		newPosition= louie.CompPosition:TranslateWorldToLocal(newPosition);
		louie.CompPosition:SetPositionLocalY(newPosition.y);

		--if box can be broken, jump
		if(louie.BreakBox(layer, tx, ty))then
			louie.BoxJump()
		end

		--Update variables
		louie.angle=newAngle;
		louie.debug_tcolx=tx;
		louie.debug_tcoly=ty;
		louie.CompPosition:SetParent(louie.parent);
		louie.platformVelocityX=0;
		louie.platformVelocityY=0;
	end

	function louie.OnTileRight(newPosition, layer, tx, ty)
		if(louie.currentState == louie.c.STATE_ROLL)then
			--if box can be broken, Don't stop momentum
			if(louie.BreakBox(layer, tx, ty))then
				return
			end
		end

		newPosition= louie.CompPosition:TranslateWorldToLocal(newPosition);
		louie.CompPosition:SetPositionLocalX(newPosition.x);

		louie.xspd=0; --for when in the air
		louie.groundSpeed=0;
	end

	function louie.OnTileLeft(newPosition, layer, tx, ty)
		if(louie.currentState == louie.c.STATE_ROLL)then
			--if box can be broken, Don't stop momentum
			if(louie.BreakBox(layer, tx, ty))then
				return
			end
		end

		newPosition= louie.CompPosition:TranslateWorldToLocal(newPosition);
		louie.CompPosition:SetPositionLocalX(newPosition.x);


		louie.xspd=0; --for when in the air
		louie.groundSpeed=0;
	end

	function louie.OnTileUp(newPosition, layer, tx, ty)
		if (louie.yspd<0) then
			louie.BreakBox(layer, tx, ty)
			louie.CompPosition:TranslateWorldToLocal(newPosition);
			louie.CompPosition:SetPositionLocalY(newPosition.y);
			louie.yspd=0;
		end
		louie.CompSprite:SetAnimationSpeed(louie.mainSpriteID, 0);
	end

	function louie.TakeDamage(hitpoints)
		louie.xspd = louie.c.KNOCKBACK_SPEED_X * louie.facingDir*-1;
		louie.yspd = louie.c.KNOCKBACK_SPEED_Y;
		louie.attackLock=true;
		louie.LockInput(30);

		louie.health = louie.health-hitpoints;
	end

	function louie.Attacked(damage)
		if(louie.currentState ~= louie.c.STATE_ROLL)then--if not rolling
			louie.TakeDamage(damage);
			return true --was hit
		end

		return false; --not hit
	end

	function louie.OnLuaEvent(senderEID, eventString)

	end

	louie.EntityInterface = {
		IsSolid		= function ()		return true; end,
		GetHealth = function ()			return louie.health; end,
		Attack		= function (damage) return louie.Attacked(damage); end
	}

	return louie
end

return container.NewLouie
