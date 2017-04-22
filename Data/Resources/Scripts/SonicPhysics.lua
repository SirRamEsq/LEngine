package.path = package.path .. ";/opt/zbstudio/lualibs/mobdebug/?.lua"
--require('mobdebug').start()

--Problem occurs when both horizontal and vertical ground collision lines register a collision and the mode rappidly switches back and forth
--hhorizontal registers <45 and vertical registers > 45

--Constants
	TILEWIDTH=16;
	TILEHEIGHT=16;
	
	MAXSPEED=16;
	
	WIDTH=32; --WIDTH of object sprite
	HEIGHT=32; --HEIGHT of object sprite
	
	GRAVITY=0.21875;
	JUMPHEIGHT=-6.5;
	ACCELERATION=.046875;
	ACCELERATION_AIR=ACCELERATION*2;
	DEACCELERATION=.5;
	ACCELERATION_TOP=6; --Max Speed Heor can acheive through normal acceleration
  GROUNDSPDFALL=2.5--2.5;   --Value at which you lose the grip on the wall
	
	FRICTION_AIR=.2;
	FRICTION_MODIFER=.046875; --The friction of a tile is multiplied by this constant to get the actual friction value
  MAX_STEPUP_PIXELS = 14;
  
  MODE_THRESHOLD_DR=50; --sonic uses 45
  MODE_THRESHOLD_DL=310;--sonic uses 315
  MODE_THRESHOLD_UL=0;
  MODE_THRESHOLD_UR=0;
  
  debug_tcolx=0;
  debug_tcoly=0;
	
--Variables
	--General Movement Variables
		xspd=0;
		yspd=0;
		groundSpeed=0;
    prevGroundSpeed=0; -- Ground speed of previous frame
		tileFriction=0.046875;
		groundTouch=false;
		prevGroundTouch=false; --whether ground was touched on the previous frame
		inputLock=false;
		lockTimer=0;
		hasShield=true;
		shieldEID=0;
		angle=0;
	
	--Input booleans
		inputU=false;
		inputD=false;
		inputL=false;
		inputR=false;
		inputThrow=false;
    
	--C++ Interfacing
		CPPInterface=nil;
		mySprite=nil;
		mySpriteComp=nil;
		myColComp=nil;
		EID=0;
		depth=0;
		parent=0;
    
  --Modes
		--The ID Numbers for the modes
		ROT_ID_DOWN = 0; --using the bottom colbox for terrain
		ROT_ID_LEFT = 1; --using the left colbox for terrain
		ROT_ID_RIGHT= 2; --using the right colbox for terrain
		ROT_ID_UP   = 3; --using the top colbox for terrain
    
  --Height Maps
		HORIZONTAL= 0;
		VERTICAL  = 1;
		
		--The angle the sprite must rotated at for each angle
		ROT_ANGLE_DOWN  = 0;
		ROT_ANGLE_LEFT  = 270;
		ROT_ANGLE_RIGHT = 90;
		ROT_ANGLE_UP    = 180;
    
    --Rotation Mode Class
    RotationMode = {};
    RotationMode.__index = RotationMode;
    function RotationMode.new(id, angle, orient)
      local rotMode={};
      setmetatable(rotMode,RotationMode);
      rotMode.mID=id;
      rotMode.mAngle=angle
      rotMode.mOrient=orient;
      return rotMode;
    end
    
    ROT_MODE_DOWN  = RotationMode.new(ROT_ID_DOWN,  ROT_ANGLE_DOWN,  HORIZONTAL);
    ROT_MODE_LEFT  = RotationMode.new(ROT_ID_LEFT,  ROT_ANGLE_LEFT,  VERTICAL);
    ROT_MODE_RIGHT = RotationMode.new(ROT_ID_RIGHT, ROT_ANGLE_RIGHT, VERTICAL);
    ROT_MODE_UP    = RotationMode.new(ROT_ID_UP,    ROT_ANGLE_UP,    HORIZONTAL);
    
    rotationMode=ROT_MODE_DOWN;

--Collision boxes and ID Numbers
		--PrimaryBox
		CBOX_PRIME=0;
		
		--Entity collision
		CBOX_ENT_UP=1;
		CBOX_ENT_LEFT=2;
		CBOX_ENT_RIGHT=3;
		CBOX_ENT_DOWN=4;
		
		CBOX_TILE_UP=5;
		CBOX_TILE_LEFT=6;
		CBOX_TILE_RIGHT=7;
		CBOX_TILE_DOWN_A=8;
		CBOX_TILE_DOWN_B=9;
    
    --Variables
    CBOX_WIDTH  = 32;
    CBOX_HEIGHT = 32;
    
    --Where CBOXs are relative to ground (Down) position
    --Should check horizontal boxes first
    CBOX_GROUND_A_X_OFFSET =  9;
    CBOX_GROUND_B_X_OFFSET =  CBOX_WIDTH-9;
    CBOX_GROUND_Y_OFFSET   =  CBOX_HEIGHT/2;
    CBOX_GROUND_H_OFFSET   =  CBOX_HEIGHT-CBOX_GROUND_Y_OFFSET+20;
    CBOX_GROUND_ORDER      =  5;
    
    CBOX_RIGHT_X_OFFSET    =  CBOX_WIDTH-7;
    CBOX_RIGHT_Y_OFFSET    =  20; --Formerly 20
    CBOX_RIGHT_W_OFFSET    =  -1;
    CBOX_RIGHT_H_OFFSET    =  1;--(CBOX_HEIGHT/2);
    CBOX_RIGHT_ORDER       =  15;
    
    CBOX_LEFT_X_OFFSET     =  7;
    CBOX_LEFT_Y_OFFSET     =  20; --Formerly 20
    CBOX_LEFT_W_OFFSET     =  1;
    CBOX_LEFT_H_OFFSET     =  1;--(CBOX_HEIGHT/2);
    CBOX_LEFT_ORDER        =  15;
    
    CBOX_UP_Y_OFFSET       =  15;
    CBOX_UP_H_OFFSET       =  -1;
    CBOX_UP_W_OFFSET       =  16
    CBOX_UP_X_OFFSET       =  (CBOX_WIDTH/2)-(CBOX_UP_W_OFFSET/2);
    CBOX_UP_ORDER          =  5;
    
    CBOX_DOWN_Y_OFFSET       =  CBOX_HEIGHT-10;
    CBOX_DOWN_H_OFFSET       =  1;
    CBOX_DOWN_W_OFFSET       =  16
    CBOX_DOWN_X_OFFSET       =  (CBOX_WIDTH/2)-(CBOX_UP_W_OFFSET/2);
    

--Collision Boxes
		--Entity Collision Boxes
			cboxPrimary=nil;
			
			cboxEntRight=nil;
			cboxEntLeft=nil;
			cboxEntUp=nil;
			cboxEntDown=nil;
		
		--Horizontal Tile Collision Boxes
			cboxHTileRight=nil;
			cboxHTileLeft=nil;
			cboxHTileUp=nil;
      cboxHTileDown=nil;
		
		--Ground Checking Collision Boxes
			cboxTileDownGroundA=nil;
			cboxTileDownGroundB=nil;

		--Collision state variables
			highestHeight=0;
      lowestAngle=0;
			firstCollision=false;

--Camera
	cameraValues= {};
	cameraValues.x= {};
	cameraValues.y= {};
	cameraValues.saveIndex=0;
	cameraValues.loadIndex=0;
	CAMERA_BUFFER=4;
  
  
  line1=0;
  line2=0;
  line3=0;
  line4=0;
function Init(LInterface, DEPTH, par)
	--------------------
	--LUA Verion Check--
	--------------------
	if (_VERSION == "Lua 5.1") then
		LUA51=true;
	else
		LUA51=false;
	end
	-----------------------
	--C++ Interface setup--
	-----------------------
  
  --mobdebug.start();
	
	depth=DEPTH;
	parent=par;
	CPPInterface=LInterface;
	EID=CPPInterface:GetEID();
	
	CPPInterface:WriteError("Parent is: " .. parent);
	CPPInterface:ListenForInput("up", EID);
	CPPInterface:ListenForInput("down", EID);
	CPPInterface:ListenForInput("left", EID);
	CPPInterface:ListenForInput("right", EID);
	CPPInterface:ListenForInput("throw", EID);

	CPPInterface:WriteError(depth);

	
	CPPInterface:AddSpriteComponent();
	CPPInterface:AddCollisionComponent();
	myColComp=CPPInterface:GetCollisionComp();
	mySpriteComp=CPPInterface:GetSpriteComp();
	
	----------------
	--Sprite setup--
	----------------
	mySpriteComp:SetRotation(0);
	mySprite=CPPInterface:LoadSprite("HeorSPR");
	if(mySprite==nil) then
		CPPInterface:WriteError("sprite is NIL");
	else
		mySprite:AddAnimation("HeorShield");
		mySprite:AddAnimation("HeorNoShield");
		mySprite:AddAnimation("HeorShieldStand");
		mySprite:AddAnimation("HeorNoShieldStand");

		aniRect=CRect(0,0,WIDTH,HEIGHT);
	
		for i=0, 6 do
			mySprite:AppendImage("HeorShield",aniRect,"HeorFull.png");
			aniRect.x= aniRect.x + WIDTH;
		end
		
		aniRect.x= 0;
		aniRect.y= aniRect.y + HEIGHT;
		mySprite:AppendImage("HeorShieldStand",aniRect,"HeorFull.png");
		
		aniRect.x= 0;
		aniRect.y= aniRect.y + HEIGHT;
		for i=0, 6 do
			mySprite:AppendImage("HeorNoShield",aniRect,"HeorFull.png");
			aniRect.x= aniRect.x + WIDTH;
		end
		
		aniRect.x= 0;
		aniRect.y= aniRect.y + HEIGHT;
		mySprite:AppendImage("HeorNoShieldStand",aniRect,"HeorFull.png");
		
		mySpriteComp:SetSprite(mySprite);
		mySprite=mySpriteComp:GetSprite();
		mySprite:SetSpeed(.2);
		mySprite:SetColorKey("HeorShield",0,255,255,255);
		mySprite:SetCurrentAnimation("HeorShield");
	end
	
	mySpriteComp:SetDepth(depth);
  --[[
  ----------------
	--Sprite setup--SONIC
	----------------
	SONIC_WIDTH=40;
  SONIC_HEIGHT=40;
	mySpriteComp:SetRotation(0);
	mySprite=CPPInterface:LoadSprite("SonicSPR");
	if(mySprite==nil) then
		CPPInterface:WriteError("sprite is NIL");
	else
		mySprite:AddAnimation("HeorShield");
		mySprite:AddAnimation("HeorShieldStand");
		mySprite:AddAnimation("HeorNoShield");
		mySprite:AddAnimation("HeorNoShieldStand");

		aniRect=CRect(0,0,SONIC_WIDTH,SONIC_HEIGHT);
	
		for i=0, 12 do
			mySprite:AppendImage("HeorNoShield",aniRect,"sonicAdjusted.png");
			aniRect.x= aniRect.x + SONIC_WIDTH;
		end
		
		aniRect.x= 0;
		aniRect.y= aniRect.y + SONIC_HEIGHT;
		mySprite:AppendImage("HeorShieldStand",aniRect,"sonicAdjusted.png");
    mySprite:AppendImage("HeorNoShieldStand",aniRect,"sonicAdjusted.png");
		
		aniRect.x= 0;
		aniRect.y= aniRect.y + SONIC_HEIGHT;
		for i=0, 3 do
			mySprite:AppendImage("HeorShield",aniRect,"sonicAdjusted.png");
			aniRect.x= aniRect.x + SONIC_WIDTH;
		end
		
		mySpriteComp:SetSprite(mySprite);
		mySprite=mySpriteComp:GetSprite();
		mySprite:SetSpeed(.2);
		mySprite:SetColorKey("HeorShield",0,0,128,128);
		mySprite:SetCurrentAnimation("HeorShield");
	end
	
	mySpriteComp:SetDepth(depth);]]--

	
	-----------------------
	--Collision for tiles--
	-----------------------
	
  --Boxes are to the 'right', 'left', 'up', and 'down' in the absolute sense, not relative to rotation mode or motion
	cboxHTileRight=	  CRect(CBOX_RIGHT_X_OFFSET,	CBOX_RIGHT_Y_OFFSET,	CBOX_RIGHT_W_OFFSET,	CBOX_RIGHT_H_OFFSET);
	cboxHTileLeft=	  CRect(CBOX_LEFT_X_OFFSET,   CBOX_LEFT_Y_OFFSET,   CBOX_LEFT_W_OFFSET, 	CBOX_LEFT_H_OFFSET );
	cboxHTileUp=		  CRect(CBOX_UP_X_OFFSET,   	CBOX_UP_Y_OFFSET,	    CBOX_UP_W_OFFSET,     CBOX_UP_H_OFFSET   );
  cboxHTileDown=	  CRect(CBOX_DOWN_X_OFFSET,   CBOX_DOWN_Y_OFFSET,	  CBOX_DOWN_W_OFFSET,   CBOX_DOWN_H_OFFSET );
  
  cboxVTileRight=	  CRect(CBOX_UP_Y_OFFSET,	      CBOX_UP_X_OFFSET,	      CBOX_UP_W_OFFSET,	     CBOX_UP_H_OFFSET   );
	cboxVTileLeft=	  CRect(-(CBOX_UP_Y_OFFSET),	  CBOX_UP_X_OFFSET,	      -CBOX_UP_W_OFFSET,	   CBOX_UP_H_OFFSET   );
	cboxVTileUp=		  CRect(CBOX_RIGHT_Y_OFFSET,   	CBOX_RIGHT_X_OFFSET,	  CBOX_RIGHT_H_OFFSET,   -CBOX_RIGHT_W_OFFSET);
  cboxVTileDown=		CRect(CBOX_LEFT_Y_OFFSET,   	CBOX_LEFT_X_OFFSET,	    CBOX_LEFT_H_OFFSET,    CBOX_LEFT_W_OFFSET );
	
	cboxTileDownGroundA=	CRect(CBOX_GROUND_A_X_OFFSET,	CBOX_GROUND_Y_OFFSET, 	1,	CBOX_GROUND_H_OFFSET);
	cboxTileDownGroundB=	CRect(CBOX_GROUND_B_X_OFFSET,	CBOX_GROUND_Y_OFFSET, 	1,  CBOX_GROUND_H_OFFSET);
  
  cboxTileLeftGroundA=	CRect((CBOX_WIDTH-CBOX_GROUND_Y_OFFSET),	CBOX_GROUND_A_X_OFFSET,  -CBOX_GROUND_H_OFFSET, 1);
	cboxTileLeftGroundB=	CRect((CBOX_WIDTH-CBOX_GROUND_Y_OFFSET),	CBOX_GROUND_B_X_OFFSET,  -CBOX_GROUND_H_OFFSET, 1);
  
  cboxTileRightGroundA=	CRect(CBOX_GROUND_Y_OFFSET,	CBOX_GROUND_A_X_OFFSET, 	CBOX_GROUND_H_OFFSET, 1);
	cboxTileRightGroundB=	CRect(CBOX_GROUND_Y_OFFSET,	CBOX_GROUND_B_X_OFFSET, 	CBOX_GROUND_H_OFFSET, 1);
  
  cboxTileUpGroundA=CRect(CBOX_GROUND_A_X_OFFSET,	CBOX_HEIGHT-CBOX_GROUND_Y_OFFSET, 	1,	-(CBOX_GROUND_H_OFFSET) );
	cboxTileUpGroundB=CRect(CBOX_GROUND_B_X_OFFSET,	CBOX_HEIGHT-CBOX_GROUND_Y_OFFSET, 	1,	-(CBOX_GROUND_H_OFFSET) );
	
	cboxPrimary=	CRect(5,	5, 		CBOX_WIDTH-10,	CBOX_HEIGHT-10);
	
	myColComp:AddCollisionBox(cboxPrimary, CBOX_PRIME, 0);
	myColComp:CheckForEntities(CBOX_PRIME);
	myColComp:SetPrimaryCollisionBox(CBOX_PRIME, false);
	
	myColComp:AddCollisionBox(cboxTileDownGroundA, CBOX_TILE_DOWN_A, CBOX_GROUND_ORDER);
	myColComp:CheckForTiles(CBOX_TILE_DOWN_A);
	
	myColComp:AddCollisionBox(cboxTileDownGroundB, CBOX_TILE_DOWN_B, CBOX_GROUND_ORDER);
	myColComp:CheckForTiles(CBOX_TILE_DOWN_B);
	
	myColComp:AddCollisionBox(cboxHTileRight, CBOX_TILE_RIGHT, CBOX_RIGHT_ORDER);
	myColComp:CheckForTiles(CBOX_TILE_RIGHT);
	
	myColComp:AddCollisionBox(cboxHTileLeft, CBOX_TILE_LEFT, CBOX_LEFT_ORDER);
	myColComp:CheckForTiles(CBOX_TILE_LEFT);
	
	myColComp:AddCollisionBox(cboxHTileUp, CBOX_TILE_UP, CBOX_UP_ORDER);
	myColComp:CheckForTiles(CBOX_TILE_UP);
	
	groundTouch=true;

--Init Camera
	for i=0, CAMERA_BUFFER-1 do
		cameraValues.x[i]=CPPInterface:GetX()-225;
		cameraValues.y[i]=CPPInterface:GetY()-160;
	end
	cameraValues.saveIndex=CAMERA_BUFFER-1;
	cameraValues.loadIndex=0;
  
  --------------------
  --Particle Effects--
  --------------------
  --[[
  particleCreatorID=CPPInterface:NewParticleCreator(800, 400);
  particleCreator=CPPInterface:GetParticleCreator(particleCreatorID);
  
  local particlePositionMin = Coord2df(CPPInterface:GetX()-20,CPPInterface:GetY()-10);
  local particlePositionMax = Coord2df(CPPInterface:GetX(),   CPPInterface:GetY()+10);
  
  local particleVelocityMin = Coord2df(-0.25,-8.25);
  local particleVelocityMax = Coord2df( 0.25,-7.75);
  
  local particleAccelMin= Coord2df(-0.015, 0.08);
  local particleAccelMax= Coord2df( 0.015, 0.08);
  
  local particleRotationMin= 0.0;
  local particleRotationMax= 360.0;
  
  particleCreator:SetPosition(particlePositionMin, particlePositionMax);
  particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax);
  particleCreator:SetAcceleration(particleAccelMin, particleAccelMax);
  particleCreator:SetSprite(mySprite);
  particleCreator:SetDepth(200);
  particleCreator:SetParticlesPerFrame(1);
  particleCreator:SetColor(0,0,0,0.25,  1,1,1,1);
  particleCreator:SetScalingX(0.75, 1.25);
  particleCreator:SetScalingY(0.75, 1.25);
  particleCreator:SetRotation(particleRotationMin, particleRotationMax);--]]
  
  line1=CPPInterface:RenderLine(0,0,16,16);
  line2=CPPInterface:RenderLine(0,0,16,16);
  
  line3=CPPInterface:RenderLine(0,0,16,16);
  CPPInterface:LineChangeColor(line3,0,255,0);
  line4=CPPInterface:RenderLine(0,0,16,16);
  CPPInterface:LineChangeColor(line4,0,255,0);
  
  line5=CPPInterface:RenderLine(0,0,16,16);
  CPPInterface:LineChangeColor(line5,0,0,255);
  line6=CPPInterface:RenderLine(0,0,16,16);
  CPPInterface:LineChangeColor(line6,0,0,255);
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

function OnKeyDown(keyname)
	if(keyname=="up") then
		inputU=true;
	elseif(keyname=="down") then
		inputD=true;
	elseif(keyname=="left") then
		inputL=true;
	elseif(keyname=="right") then
		inputR=true;
	elseif(keyname=="throw") then
		inputThrow=true;
	end
end

function OnKeyUp(keyname)
	if(keyname=="up") then
		inputU=false;
		if(yspd<(-4))then
			yspd=(-4);
		end
	elseif(keyname=="down") then
		inputD=false;
	elseif(keyname=="left") then
		inputL=false;
	elseif(keyname=="right") then
		inputR=false;
	elseif(keyname=="throw") then
		inputThrow=false;
	end
end

--uses buffer. Camera follows instead of being exact
function MoveCamera(xxx, yyy)
	cameraValues.x[cameraValues.saveIndex]=xxx;
	cameraValues.y[cameraValues.saveIndex]=yyy;
	
	CPPInterface:MoveCameraXAbs(cameraValues.x[cameraValues.loadIndex]);
	--CPPInterface:MoveCameraYAbs(cameraValues.y[cameraValues.loadIndex]);
	CPPInterface:MoveCameraYAbs(yyy);

	cameraValues.loadIndex = cameraValues.loadIndex + 1;
	cameraValues.saveIndex = cameraValues.saveIndex + 1;
	if(cameraValues.loadIndex>CAMERA_BUFFER -1)then
		cameraValues.loadIndex=0;
	end
	if(cameraValues.saveIndex>CAMERA_BUFFER -1)then
		cameraValues.saveIndex=0;
	end
end
function SetModeLeft()
	myColComp:ChangeBox(CBOX_TILE_DOWN_A, cboxTileLeftGroundA);
	myColComp:ChangeBox(CBOX_TILE_DOWN_B, cboxTileLeftGroundB);
	myColComp:ChangeBox(CBOX_TILE_LEFT, cboxVTileUp);
	myColComp:ChangeBox(CBOX_TILE_UP, cboxVTileRight);
	myColComp:ChangeBox(CBOX_TILE_RIGHT, cboxVTileDown);
	rotationMode=ROT_MODE_LEFT;
end

function SetModeRight()
	myColComp:ChangeBox(CBOX_TILE_DOWN_A, cboxTileRightGroundA);
	myColComp:ChangeBox(CBOX_TILE_DOWN_B, cboxTileRightGroundB);
	myColComp:ChangeBox(CBOX_TILE_LEFT, cboxVTileDown);
	myColComp:ChangeBox(CBOX_TILE_UP, cboxVTileLeft);
	myColComp:ChangeBox(CBOX_TILE_RIGHT, cboxVTileUp);
	rotationMode=ROT_MODE_RIGHT;
end

function SetModeUp()
  myColComp:ChangeBox(CBOX_TILE_DOWN_A, cboxTileUpGroundA);
	myColComp:ChangeBox(CBOX_TILE_DOWN_B, cboxTileUpGroundB);
	myColComp:ChangeBox(CBOX_TILE_LEFT, cboxHTileRight);
	myColComp:ChangeBox(CBOX_TILE_UP, cboxHTileDown);
	myColComp:ChangeBox(CBOX_TILE_RIGHT, cboxHTileLeft);
	rotationMode=ROT_MODE_UP;
end

function SetModeDown()
	myColComp:ChangeBox(CBOX_TILE_DOWN_A, cboxTileDownGroundA);
	myColComp:ChangeBox(CBOX_TILE_DOWN_B, cboxTileDownGroundB);
	myColComp:ChangeBox(CBOX_TILE_LEFT,  cboxHTileLeft);
	myColComp:ChangeBox(CBOX_TILE_UP,    cboxHTileUp);
	myColComp:ChangeBox(CBOX_TILE_RIGHT, cboxHTileRight);
  rotationMode=ROT_MODE_DOWN;
end

function RotateModeClockWise()
  if(rotationMode.mID==ROT_ID_DOWN)then
    SetModeLeft();
  elseif(rotationMode.mID==ROT_ID_LEFT)then
    SetModeUp();
  elseif(rotationMode.mID==ROT_ID_RIGHT)then
    SetModeDown();
  elseif(rotationMode.mID==ROT_ID_UP)then
    SetModeRight();
  end
end

function RotateModeCounterClockWise()
  if(rotationMode.mID==ROT_ID_DOWN)then
    SetModeRight();
  elseif(rotationMode.mID==ROT_ID_LEFT)then
    SetModeDown();
  elseif(rotationMode.mID==ROT_ID_RIGHT)then
    SetModeUp();
  elseif(rotationMode.mID==ROT_ID_UP)then
    SetModeLeft();
  end
end

function LockInput(frames)
	lockTimer=frames;
	inputLock=true;
end

function UnlockInput()
	inputLock=false;
	lockTimer=0;
end

function UpdateLock()
	if(lockTimer>0)then
		lockTimer= lockTimer - 1;
	end
	if(lockTimer<=0)then --Use <= instead of == in this scenario; it's better practice as a "just in case"
		inputLock=false;
	end
end

function Update()
	-------------------------------------------------
	--Complete ground Collision from Previous frame--
	-------------------------------------------------
	
	MoveCamera(CPPInterface:GetX()-225, CPPInterface:GetY()-160);
  
  local angleSigned=AngleToSignedAngle(angle);
	
	if(groundTouch)then
		if(not prevGroundTouch) then
      if( math.abs(angleSigned)<25 )then
        if(angleSigned>=0)then
          groundSpeed=xspd;
        else
          groundSpeed=xspd;
        end
      elseif( math.abs(angleSigned)<=45 ) then
        if(math.abs(xspd) > math.abs(yspd))then
          groundSpeed=xspd;
        elseif(angleSigned>0)then
          groundSpeed=yspd*-1*0.5*math.abs((math.cos(math.rad(angle))));
        else
          groundSpeed=yspd*0.5*math.abs((math.cos(math.rad(angle))));
        end
			
      else
        if(angleSigned>0)then
          groundSpeed=yspd*-1--*math.abs((math.sin(math.rad(angle))));
        else
          groundSpeed=yspd--*math.abs((math.sin(math.rad(angle))));
        end  
      end
      yspd=0;
		end
  end
  
  --if you change direction from being on a steep slope, lock in the input for half a second
  --SLOPE FACTOR
  if(  ( (angle>30)and(angle<180) ) or ( (angle<330)and(angle>180) )  )then
    local slp=0.125;
    local extraSpeed=slp*math.sin(math.rad(angle))*-1;
    
    local zeroGSPD=(groundSpeed==0);
    local positiveGSPD=(groundSpeed>0);
    local newGSPD;	
    groundSpeed=groundSpeed+extraSpeed;
    newGSPD=(groundSpeed>0);
    if( (positiveGSPD~=newGSPD) and (not zeroGSPD) )then
      LockInput(30);
    end
  end
	
	local friction; --Friction value used for this frame
	friction= tileFriction--FRICTION_MODIFER * tileFriction
  
  --If in the air, set mode to down
  if (not groundTouch) then
		if(rotationMode.mID~=ROT_ID_DOWN) then
			SetModeDown();
		end
    yspd= yspd+GRAVITY;
    mySpriteComp:SetRotation(0);
    angle=0;
  end
	
	----------------
	--Handle Input--
	----------------
	
	if ( (inputU) and (groundTouch) ) then
    if(math.abs(angleSigned)<25)then
      yspd=JUMPHEIGHT;
    else
      yspd=JUMPHEIGHT * math.abs(math.cos(math.rad(angle)));
      xspd=JUMPHEIGHT * math.sin(math.rad(angle));
      --yspd=JUMPHEIGHT;
    end
  
		groundSpeed=0;
    angle=0;
		groundTouch=false;
		SetModeDown();
		UnlockInput();
	end
	if ( (inputR) and (not inputLock) ) then
		if( (groundSpeed<ACCELERATION_TOP) and (groundTouch) ) then
			if(groundSpeed<0) then --Add friction to the ground speed (slowing him faster) if moving left
				groundSpeed= groundSpeed + (DEACCELERATION);
			else
				groundSpeed= groundSpeed + ACCELERATION;
			end
		end
		if(not groundTouch) then
			if(xspd<ACCELERATION_TOP) then
				xspd= xspd + ACCELERATION_AIR;
			end
			if(xspd<0) then
				xspd= xspd + ACCELERATION_AIR + FRICTION_AIR;
			end
		end
	end
	if ( (inputL) and (not inputLock) ) then
		if( (groundSpeed>-ACCELERATION_TOP) and (groundTouch) ) then
			if(groundSpeed>0) then --Add friction to the ground speed (slowing him faster) if moving right
				groundSpeed= groundSpeed - (DEACCELERATION);
			else
				groundSpeed= groundSpeed - ACCELERATION;
			end
		end
		if(not groundTouch) then
			if(xspd>-ACCELERATION_TOP) then
				xspd= xspd - ACCELERATION_AIR;
			end
			if(xspd>0) then
				xspd= xspd - ACCELERATION_AIR - FRICTION_AIR;
			end
		end
	end
	
	----------------------
	--Slow With Friction--
	----------------------
	if ( ((not inputL) and (not inputR)) or (inputLock) )then
		if(groundTouch)then
			if(groundSpeed>0)then
				if(friction>=groundSpeed)then
					groundSpeed=0;
				else
					groundSpeed= groundSpeed - friction;
				end
		
			elseif(groundSpeed<0)then
				if(groundSpeed>=(friction*-1) )then
					groundSpeed=0;
				else
					groundSpeed= groundSpeed + friction;
				end
			end
		else
			if(xspd>0)then
				if(FRICTION_AIR>=xspd)then
					xspd=0;
				else
					xspd= xspd - FRICTION_AIR;
				end
		
			elseif(xspd<0)then
				if(xspd>=(FRICTION_AIR*-1))then
					xspd=0;
				else
					xspd= xspd + FRICTION_AIR;
				end
			end
		end
	end

	----------------
	--Shield Throw--
	----------------
	
	if( (inputThrow) and (hasShield) ) then
		t={};
		t["test"]=31337;
		t["tes2"]=1337;
		--CPPInterface:WriteError("Type is: " .. type(t));
		shieldEID= CPPInterface:NewEntity('shield.lua', CPPInterface:GetX()+WIDTH, CPPInterface:GetY()+(HEIGHT/4), depth, EID);
		hasShield=false;
  end
  
  if(inputD)then
    if(groundSpeed<0)then
      groundSpeed=-MAXSPEED;
    else
      groundSpeed=MAXSPEED;
    end
	end
	
	-----------
	--Animate--
	-----------
	
	local newImgSpd=math.abs(groundSpeed)/16;
	if(newImgSpd>2)then
		newImgSpd=2;
	end

	if(groundSpeed>ACCELERATION)then
		if(hasShield)then
			mySprite:SetCurrentAnimation("HeorShield");
		else
			mySprite:SetCurrentAnimation("HeorNoShield");
		end
		mySprite:SetSpeed(newImgSpd);
		mySpriteComp:SetScalingX(1);
		
	elseif(groundSpeed<-(ACCELERATION))then
		if(hasShield)then
			mySprite:SetCurrentAnimation("HeorShield");
		else
			mySprite:SetCurrentAnimation("HeorNoShield");
		end
		mySprite:SetSpeed(newImgSpd);
		mySpriteComp:SetScalingX(-1);
		
	elseif(groundSpeed==0)then
		if(hasShield)then
			mySprite:SetCurrentAnimation("HeorShieldStand");
		else
			mySprite:SetCurrentAnimation("HeorNoShieldStand");
		end
		mySprite:SetSpeed(0);
	end
  
  --Why myspritecomp and not mysprite?
  if(groundTouch)then
      mySpriteComp:SetRotation(-angle);
      --mySpriteComp:SetRotation(-angle-rotationMode.mAngle);
  else
    mySpriteComp:SetRotation(0);
  end
  
  --------------------------
	--Change mode if need be--
	--------------------------
  local tooSlow=( (groundSpeed<GROUNDSPDFALL) and (groundSpeed>-GROUNDSPDFALL) );
  if(rotationMode.mOrient==HORIZONTAL)then
		if( (groundSpeed<GROUNDSPDFALL) and (groundSpeed>-GROUNDSPDFALL) and (rotationMode.mID~=ROT_ID_DOWN) )then
			SetModeDown();
			LockInput(30);
		elseif( (angle>MODE_THRESHOLD_DR)and(angle<180) --[[and (groundSpeed>=0)]] )then
			RotateModeCounterClockWise();
      --CPPInterface:MoveX((math.floor(CPPInterface:GetXInt())+1)*16);
		elseif( (angle<MODE_THRESHOLD_DL)and(angle>180) --[[and (groundSpeed<=0)]] )then
			RotateModeClockWise();
		end
	else
		if( tooSlow )then
      local isAccelerating=false;
      local speedIsPositive = groundSpeed > 0;
      if(speedIsPositive)then
        if(prevGroundSpeed<=groundSpeed)then
          isAccelerating=true;
        end
      else
        if(prevGroundSpeed>=groundSpeed)then
          isAccelerating=true;
        end
      end
      if(not isAccelerating)then
        if(rotationMode.mID==ROT_ID_RIGHT)then
          xspd=0;
          yspd=groundSpeed*math.abs(math.sin(math.rad(angle)))*-1;
        else
          xspd=0;
          yspd=groundSpeed*math.abs(math.sin(math.rad(angle)));
        end
        SetModeDown();
        groundSpeed=0;
        groundTouch=false;
        prevGroundTouch=false;--Hack1A; causes different behaviour in the tile collision function
        LockInput(15);
      end
    elseif( (angle<MODE_THRESHOLD_DR) )then
      if(rotationMode.mID==ROT_ID_RIGHT)then
        RotateModeClockWise();
      end
    elseif( (angle>MODE_THRESHOLD_DL) )then
      if(rotationMode.mID==ROT_ID_LEFT)then
        RotateModeCounterClockWise();
      end
    end
  end
  
	----------------------
	--Set X and Y Speeds--
	----------------------
	local speedDampenX=1;
  local speedDampenY=1;
  
	if(groundSpeed>MAXSPEED) then
		groundSpeed=MAXSPEED;
	elseif(groundSpeed<(-MAXSPEED)) then
		groundSpeed=-MAXSPEED;
	end
  
  if(xspd>MAXSPEED) then
		xspd=MAXSPEED;
	elseif(xspd<(-MAXSPEED)) then
		xspd=-MAXSPEED;
	end
  
    if(yspd>MAXSPEED) then
		yspd=MAXSPEED;
	elseif(yspd<(-MAXSPEED)) then
		yspd=-MAXSPEED;
	end
  
  if(groundTouch) then
    if(rotationMode==ROT_MODE_DOWN)then --WORKS
      xspd=groundSpeed;
      yspd=0;
      speedDampenX=math.cos(math.rad(angle));
      
    elseif(rotationMode==ROT_MODE_LEFT)then
      xspd=0;
      yspd=groundSpeed;
      speedDampenY=math.abs(math.cos(math.rad(angle - rotationMode.mAngle)));
      
    elseif(rotationMode==ROT_MODE_RIGHT)then --WORKS
      xspd=0;
      yspd=-(groundSpeed);
      speedDampenY=math.abs(math.cos(math.rad(angle - rotationMode.mAngle)));
      
    elseif(rotationMode==ROT_MODE_UP)then --IN PROGRESS
      xspd=(-groundSpeed);
      yspd=0;
      speedDampenX=math.abs(math.cos(math.rad(angle)));
    end
	end
  
  if (not groundTouch)then
    speedDampenX=1;
    speedDampenY=1;
  end
  
  -------------------
	--Update CPP Data--
	-------------------
	CPPInterface:MovePos(xspd*speedDampenX, yspd*speedDampenY);
	
	-----------------------------------------
	--Prepare for TCollision and Next Frame--
	-----------------------------------------
  local buffer=0;
  if(groundTouch)then
    buffer=16;
  end
	--myColComp:ChangeHeight(CBOX_TILE_DOWN_A, math.floor(yspd + 0.5 + CBOX_HEIGHT - CBOX_GROUND_Y_OFFSET));
	--myColComp:ChangeHeight(CBOX_TILE_DOWN_B, math.floor(yspd + 0.5 + CBOX_HEIGHT - CBOX_GROUND_Y_OFFSET));
  --myColComp:ChangeHeight(CBOX_TILE_UP, math.floor(yspd + 0.5));
	--myColComp:ChangeHeight(CBOX_TILE_DOWN_A, HEIGHT);
	--myColComp:ChangeHeight(CBOX_TILE_DOWN_B, HEIGHT);
  
  --Probably going to want to extend ground collision boxes when going really fast
	
	prevGroundTouch=groundTouch;
	groundTouch=false;
	firstCollision=false;
  
  UpdateLock();
  
  local xx=CPPInterface:GetX();
  local yy=CPPInterface:GetY();
  
  prevGroundSpeed=groundSpeed;--[[
  
  CPPInterface:LineChangePosition(line1, xx+cboxTileRightGroundA:GetLeft(), yy+cboxTileRightGroundA:GetTop(),
                                  xx+cboxTileRightGroundA:GetRight(), yy+cboxTileRightGroundA:GetBottom());
  CPPInterface:LineChangePosition(line2, xx+cboxTileRightGroundB:GetLeft(), yy+cboxTileRightGroundB:GetTop(),
                                  xx+cboxTileRightGroundB:GetRight(), yy+cboxTileRightGroundB:GetBottom());
  
  CPPInterface:LineChangePosition(line3, xx+cboxTileDownGroundA:GetLeft(), yy+cboxTileDownGroundA:GetTop(),
                                  xx+cboxTileDownGroundA:GetRight(), yy+cboxTileDownGroundA:GetBottom());
  CPPInterface:LineChangePosition(line4, xx+cboxTileDownGroundB:GetLeft(), yy+cboxTileDownGroundB:GetTop(),
                                  xx+cboxTileDownGroundB:GetRight(), yy+cboxTileDownGroundB:GetBottom());
  
  CPPInterface:LineChangePosition(line5, xx+cboxTileLeftGroundA:GetLeft(), yy+cboxTileLeftGroundA:GetTop(),
                                  xx+cboxTileLeftGroundA:GetRight(), yy+cboxTileLeftGroundA:GetBottom());
  CPPInterface:LineChangePosition(line6, xx+cboxTileLeftGroundB:GetLeft(), yy+cboxTileLeftGroundB:GetTop(),
                                  xx+cboxTileLeftGroundB:GetRight(), yy+cboxTileLeftGroundB:GetBottom());]]--
end

function CanStepUp(y, ty, hmap, boxid)
	local feetCoord = y + HEIGHT;
	local tileY = (ty+1) * TILEHEIGHT;
	local HMAP_index_value;
	local HMAPheight;
	
	if (boxid==CBOX_TILE_RIGHT) then
		HMAP_INDEX_VALUE=0;
	else
		HMAP_INDEX_VALUE=15;
	end
	HMAPheight=hmap:GetHeightMapH( HMAP_INDEX_VALUE );
	tileY= tileY - HMAPheight;
	if( tileY > feetCoord - MAX_STEPUP_PIXELS )then
		return true;
	end
	return false;
end

function OnEntityCollision(entityID, myBoxID)
	--if(entityID==shieldID)then
		hasShield=true;
		CPPInterface:DeleteEntity(entityID);
		shieldID=0;
	--end
end

function OnTileCollision(packet)
	--Get easy access to all of the packet's variables
	local boxid=packet:GetID();
	local tx=packet:GetX();
	local ty=packet:GetY();
	local layer=packet:GetLayer();
	local hmap=packet:GetHmap();
  local usesHMaps=layer:UsesHMaps();
  
	--Setup commonly used values
	local xval=CPPInterface:GetXInt();
	local yval=CPPInterface:GetYInt();
	local HMAPheight=0;
	local frameheight;
  
  --(Vertical and Horizontal are relative to mode rotation)
	local vspd;
	local hspd;
	if(rotationMode.mOrient==HORIZONTAL)then
		vspd=yspd;
		hspd=xspd;
    frameheight=ty * TILEHEIGHT;
	else
		vspd=xspd;
		hspd=yspd;
    frameheight=tx * TILEWIDTH;
	end
  
  --First collision this frame setup
	if(firstCollision==false)then
    if(((rotationMode.mID==ROT_ID_DOWN)or(rotationMode.mID==ROT_ID_RIGHT)))then
      highestHeight=frameheight + 1000;
    else
      highestHeight=frameheight - 1000;
    end
		firstCollision=true;
    lowestAngle=360;
	end

	--============================--
	--If Ground Collision Occurred--
	--============================--
	if ( ((boxid==CBOX_TILE_DOWN_A) or (boxid==CBOX_TILE_DOWN_B)) and ((groundTouch==true)or(vspd>=0)) ) then
		local cbox_value = 0;
		local HMAP_index_value= 0;
    local thisAngle=0;
    
    if(not groundTouch)then
      if(vspd<0)then
        return;
      end
      if(yval+vspd+HEIGHT<ty*TILEHEIGHT and not prevGroundTouch)then --Hack1B
        return;
      end
    end

		if(boxid==CBOX_TILE_DOWN_A) then
			cbox_value = CBOX_GROUND_A_X_OFFSET;

		elseif(boxid==CBOX_TILE_DOWN_B) then
			cbox_value = CBOX_GROUND_B_X_OFFSET;
    end
    if(rotationMode.mOrient==HORIZONTAL)then
      cbox_value= cbox_value + xval;
      HMAP_index_value= cbox_value - (tx*TILEWIDTH);
    else
      cbox_value= yval + cbox_value;
      HMAP_index_value= cbox_value - (ty*TILEHEIGHT); --cbox_value % 16
    end  
    
    if((HMAP_index_value>15)or(HMAP_index_value<0))then
        return;
    end
    
    if(rotationMode.mOrient==HORIZONTAL)then
      HMAPheight=hmap:GetHeightMapH( HMAP_index_value );
      thisAngle=hmap.angleH;
    else
      HMAPheight=hmap:GetHeightMapV( HMAP_index_value );
      thisAngle=hmap.angleH;
    end
    
    local thisAngleSigned= AngleToSignedAngle(thisAngle);
    
    if(HMAPheight==0)then
      return;
    end
    
    if((rotationMode.mID==ROT_ID_DOWN)or(rotationMode.mID==ROT_ID_RIGHT))then
      frameheight = frameheight - HMAPheight;
      if(frameheight>highestHeight)then
        return;
      end
    else
      frameheight = frameheight + HMAPheight;
      if(frameheight<highestHeight)then
        return;
      end
    end
    
    if(highestHeight==frameheight)then
      if( (math.abs(lowestAngle)<=math.abs(thisAngleSigned)) )then
        return;
      end
    end

    lowestAngle=math.abs(thisAngleSigned);
		highestHeight=frameheight;
    if(rotationMode.mOrient==HORIZONTAL)then
      if(rotationMode.mID==ROT_ID_DOWN)then
        CPPInterface:MovePosAbsY(( (ty+1) *16 ) - HEIGHT - HMAPheight);
      else
        CPPInterface:MovePosAbsY(( (ty-1) *16 ) + HEIGHT + HMAPheight);
      end
    else
      if(rotationMode.mID==ROT_ID_RIGHT)then
        CPPInterface:MovePosAbsX(( (tx+1) *16 ) - WIDTH - HMAPheight);
      else
        CPPInterface:MovePosAbsX(( (tx) *16 ) + HMAPheight);
      end
    end
		groundTouch=true;
		angle=thisAngle;
    debug_tcolx=tx;
    debug_tcoly=ty;
    if((angle==0)and(rotationMode.mID~=ROT_ID_DOWN) )then
      angle=rotationMode.mAngle;
    end

  --===========================--
	--If Right Collision Occurred--
	--===========================--
  
  elseif ( (boxid==CBOX_TILE_RIGHT) ) then
    if(usesHMaps)then
      return;
    end
		if(hspd>=0) then
      if(rotationMode.mOrient==HORIZONTAL)then
        if( CanStepUp(yval, ty, hmap, boxid)==false )then
          CPPInterface:MovePosAbsX((tx*TILEWIDTH)-CBOX_WIDTH+(CBOX_WIDTH-CBOX_RIGHT_X_OFFSET));
          xspd=0; --for when in the air
          groundSpeed=0;
          mySprite:SetSpeed(0);
          mySprite:SetCurrentAnimation("HeorShieldStand");
        end
      else --Vertical
        if(rotationMode.mID==ROT_ID_RIGHT)then
          if( CanStepUp(yval, ty, hmap, boxid)==false )then
            CPPInterface:MovePosAbsY((ty*TILEHEIGHT)-CBOX_WIDTH+(CBOX_WIDTH-CBOX_RIGHT_X_OFFSET));
            groundSpeed=0;
            mySprite:SetSpeed(0);
            mySprite:SetCurrentAnimation("HeorShieldStand");
          end
        end
      end
		end
    
  --==========================--
	--If Left Collision Occurred--
	--==========================--
	
  elseif ( (boxid==CBOX_TILE_LEFT) ) then 
    if(usesHMaps)then
      return;
    end
    if( rotationMode.mOrient == HORIZONTAL)then
      if(hspd<=0) then
        if( CanStepUp(yval, ty, hmap, boxid)==false )then
          --Subtract one because (tx+1) pushes one pixel past the actual tile colided with
          CPPInterface:MovePosAbsX( ((tx+1)*TILEWIDTH)-CBOX_LEFT_X_OFFSET-1);
          xspd=0; --for when in the air
          groundSpeed=0;
          mySprite:SetSpeed(0);
          mySprite:SetCurrentAnimation("HeorShieldStand");
        end
      end
    else
      if(rotationMode.mID==ROT_ID_RIGHT)then
        if(hspd>=0) then
          if( CanStepUp(yval, ty, hmap, boxid)==false )then
            CPPInterface:MovePosAbsX( (ty*TILEHEIGHT)-CBOX_HEIGHT+(CBOX_HEIGHT-CBOX_RIGHT_X_OFFSET));
            groundSpeed=0;
            mySprite:SetSpeed(0);
            mySprite:SetCurrentAnimation("HeorShieldStand");
          end
        end
      end
    end
    
  --=========================--
	--If Top Collision Occurred--
	--=========================--
  
elseif ( (boxid==CBOX_TILE_UP) and ((groundTouch==false)or(vspd<0))  )then
  if(rotationMode.mOrient==HORIZONTAL)then
      --Subtract one because (tx+1) pushes one pixel past the actual tile colided with
      CPPInterface:MovePosAbsY(((ty+1)*TILEHEIGHT)-1);  
      if (yspd<0) then
        yspd=0;
      end
      mySprite:SetSpeed(0);
      groundTouch=false;
  end
else
    
end
end
