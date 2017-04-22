--DEBUG
--package.path = package.path .. ";/opt/zbstudio/lualibs/mobdebug/?.lua"
--local mobdebug=require('mobdebug');

--[[
Implemented
  Platform Riding
  Roll
    Long Jump
  Wall Slide --Slightly iffy due to the crappy collision boxes
    Wall Jump
    
To Implement
  Swimming
  
Maybe...
  Grab Ledge(?)
  Attack(?)
    Super Jump from attack(?)
  
]]--

--WHAT IS UP WITH LEFT TILE COL???
--Why can you clip into the tiles by one pixel if you press left?

function NewHeor()
  heor = {}
  
  --Constants
	heor.TILEWIDTH=16;
	heor.TILEHEIGHT=16;
	
	heor.MAXSPEED=16;
  heor.MAXHEALTH=10;
	
	heor.WIDTH=16; --WIDTH of object sprite
	heor.HEIGHT=16; --HEIGHT of object sprite
	
	heor.GRAVITY=0.21875;
  heor.SLOPE_GRAVITY=0.15
	heor.JUMPHEIGHT=-6;
	heor.ACCELERATION=.046875*2;
	heor.ACCELERATION_AIR= heor.ACCELERATION;
	heor.DEACCELERATION=1;
	heor.ACCELERATION_TOP=4; --Max Speed Heor can acheive through normal acceleration
  heor.ROLL_SPEED= heor.ACCELERATION_TOP+1;
  heor.ROLL_TIMER=15;
  heor.ROLL_COOLDOWN=20;
  heor.roll_timer=0;
	
	heor.FRICTION_AIR=.1;
	heor.FRICTION_MODIFER=0.46875*2; --The friction of a tile is multiplied by this constant to get the actual friction value
  heor.MAX_STEPUP_PIXELS = 12;
  
  heor.WALLJUMP_LOCK=10;
  heor.WALLJUMP_XSPD=5;
  heor.WALLJUMP_YSPD=5;
  heor.WALLJUMP_GRAVITY=heor.GRAVITY/4;
  heor.WALLJUMP_YSPD_MAX=2;
  
  heor.STATE_NORMAL=0;
  heor.STATE_ROLL=1;
  heor.STATE_GRABLEDGE=2;
  heor.STATE_WALLSLIDE=3
  
  heor.FACING_LEFT=-1;
  heor.FACING_RIGHT=1;
  
  heor.WHIP_EID=nil;
  
  --knockback when getting hit
  heor.KNOCKBACK_SPEED_X=2.5;
  heor.KNOCKBACK_SPEED_Y=-3;
	
--Variables
	--General Movement Variables
    heor.health=heor.MAXHEALTH;
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
    heor.facingDir=heor.FACING_RIGHT;
    
    heor.ropeSwing=nil;
    
    heor.externalUseWhip=false;
    heor.externalUseWhipInterrupt=false;
    
    heor.currentState= heor.STATE_NORMAL;
	
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
		heor.mySpriteComp=nil;
    heor.myPositionComp=nil;
		heor.myColComp=nil;
		heor.EID=0;
		heor.depth=0;
		heor.parent=0;
    
  --Height Maps
		heor.HMAP_HORIZONTAL= 0;
		heor.HMAP_VERTICAL  = 1;  
    
--Collision boxes and ID Numbers
  local result=0;
  --security hole here? user can just use '..' to go wherever they want
  result, heor.collision = pcall(loadfile(commonPath .. "/collisionSystem.lua", _ENV))
      
--Sound Effects
  heor.SoundJump=nil;
  
--HUD
  heor.HUD={};
  heor.HUD.text={};
  heor.HUD.text.health=nil;

--Camera
	heor.cameraValues= {};
	heor.cameraValues.x= {};
	heor.cameraValues.y= {};
  heor.cameraValues.w=480;
  heor.cameraValues.h=320;
	heor.cameraValues.saveIndex=0;
	heor.cameraValues.loadIndex=0;
  heor.cameraValues.rect={}
  heor.cameraValues.rect.x=-16;
  heor.cameraValues.rect.y= -8;
  heor.cameraValues.rect.w= 32;
  heor.cameraValues.rect.h= 16;
	heor.CAMERA_BUFFER=5;

function heor.Initialize(self)
	-----------------------
	--C++ Interface setup--
	-----------------------
  
  --mobdebug.start()
  
	self.depth        = self.LEngineData.depth;
	self.parent       = self.LEngineData.parent;
	self.CPPInterface = self.LEngineData.interface;
	self.EID          = self.LEngineData.entityID;
	
  local EID = self.EID
	self.CPPInterface:ListenForInput(EID, "up"   );
	self.CPPInterface:ListenForInput(EID, "down" );
	self.CPPInterface:ListenForInput(EID, "left" );
	self.CPPInterface:ListenForInput(EID, "right");
	self.CPPInterface:ListenForInput(EID, "throw");
	self.CPPInterface:ListenForInput(EID, "swing");
	self.CPPInterface:ListenForInput(EID, "whip" );

  --SoundJump = CPPInterface:GetSound("smw_jump.wav");
  self.SoundJump = "smw_jump.wav";
  
  self.coinText=self.CPPInterface:RenderObjectText(EID, 400, 0, "Coins : " .. tostring(self.coinCount));
  self.coinText:SetColor(200,200,100);
  self.coinText:SetDepth(-1004);
  
  self.HUD.text.health=self.CPPInterface:RenderObjectText(EID, 190, 0, "Health : " .. tostring(self.health));
  self.HUD.text.health:SetColor(200,20,10);
  self.HUD.text.health:SetDepth(-1004);
	
	self.myColComp      = self.CPPInterface:GetCollisionComponent(EID);
	self.mySpriteComp   = self.CPPInterface:GetSpriteComponent(EID);
  self.myPositionComp = self.CPPInterface:GetPositionComponent(EID);
  
  self.myColComp:SetName(self.LEngineData.name)
  self.myColComp:SetType(self.LEngineData.objType);
	----------------
	--Sprite setup--
	----------------
	self.mySprite = self.CPPInterface:LoadSprite("SpriteLouie.xml");
  self.mySpriteB = self.CPPInterface:LoadSprite("SpriteArrow.xml");
	
  if( self.mySprite==nil ) then
		self.CPPInterface:WriteError("sprite is NIL");
  end  
		
  --Logical origin is as at the top left; (0,0) is top left
  --Renderable origin is at center;       (-width/2, -width/2) is top left
  --To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
  self.mySpriteID   = self.mySpriteComp:AddSprite(self.mySprite,  self.depth, (self.WIDTH/2), (self.HEIGHT/2)+1);
  self.mySpriteIDB  = self.mySpriteComp:AddSprite(self.mySpriteB, self.depth, -1, 2);
  self.mySpriteComp:SetAnimationSpeed(self.mySpriteIDB, 0);
  
  self.mySpriteComp:SetRotation(self.mySpriteIDB, 25);
  self.mySpriteComp:SetScalingX(self.mySpriteIDB, .75);
  self.mySpriteComp:SetScalingY(self.mySpriteIDB, .75);
  
  self.mySpriteComp:SetAnimation      (self.mySpriteID, "Stand");
  self.mySpriteComp:SetAnimationSpeed (self.mySpriteID, 1);
  self.mySpriteComp:SetRotation       (self.mySpriteID, 0);
	
	-----------------------
	--Collision for tiles--
	-----------------------
  self.collision.Init(self.WIDTH, self.HEIGHT, self.CPPInterface, self.myColComp);
  self.collision.callbackFunctions.TileUp    = self.OnTileUp;
  self.collision.callbackFunctions.TileDown  = self.OnTileDown;
  self.collision.callbackFunctions.TileLeft  = self.OnTileLeft;
  self.collision.callbackFunctions.TileRight = self.OnTileRight;

--Init Camera
	for i=0, self.CAMERA_BUFFER-1 do
    Vec2d = self.myPositionComp:GetPositionWorld();
		self.cameraValues.x[i]=Vec2d.x-225;
		self.cameraValues.y[i]=Vec2d.y-160;
	end
	self.cameraValues.saveIndex=self.CAMERA_BUFFER-1;
	self.cameraValues.loadIndex=0;
  
  local position = self.myPositionComp:GetPositionWorld();
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

function heor.OnKeyDown(self, keyname)
	if(keyname=="up") then
		self.input.key.inputU=true;
	elseif(keyname=="down") then
		self.input.key.inputD=true;
	elseif(keyname=="left") then
		self.input.key.inputL=true;
	elseif(keyname=="right") then
		self.input.key.inputR=true;
	elseif(keyname=="throw") then
		self.input.key.inputThrow=true;
  elseif(keyname=="swing") then
    self.input.key.inputSwing=true;
	elseif(keyname=="whip") then
    self.input.key.inputWhip=true;
	end
end

function heor.OnKeyUp(self, keyname)
	if(keyname=="up") then
		self.input.key.inputU=false;  
	elseif(keyname=="down") then
		self.input.key.inputD=false;
	elseif(keyname=="left") then
		self.input.key.inputL=false;
	elseif(keyname=="right") then
		self.input.key.inputR=false;
	elseif(keyname=="throw") then
		self.input.key.inputThrow=false;
  elseif(keyname=="swing") then
    self.input.key.inputSwing=false;
	elseif(keyname=="whip") then
    self.input.key.inputWhip=false;
	end
end

--uses buffer. Camera follows instead of being exact
function heor.MoveCamera(playerX, playerY)
  --[[local index=cameraValues.saveIndex-1;
  if(index<0)then
    index=CAMERA_BUFFER-1;
  end
  
  centerX=cameraValues.x[index]+(cameraValues.w/2);
  centerY=cameraValues.y[index]+(cameraValues.h/2);
  
  local boxLeft=  (centerX + cameraValues.rect.x);
  local boxRight= (centerX + cameraValues.rect.x + cameraValues.rect.w);
  local boxBottom=(centerY + cameraValues.rect.y + cameraValues.rect.h);
  local boxTop=   (centerY + cameraValues.rect.y);
                
  if(playerX < boxLeft)then
    cameraValues.x[cameraValues.saveIndex]=cameraValues.x[index]+(playerX-boxLeft);
  elseif(playerX > boxRight)then
    cameraValues.x[cameraValues.saveIndex]=cameraValues.x[index]+(playerX-boxRight);
  else
    cameraValues.x[cameraValues.saveIndex]=cameraValues.x[index];
  end
  if(playerY < boxTop)then
    cameraValues.y[cameraValues.saveIndex]=cameraValues.y[index]+(playerY-boxTop);
  elseif(playerY > boxBottom)then
    cameraValues.y[cameraValues.saveIndex]=cameraValues.y[index]+(playerY-boxBottom);
  else
    cameraValues.y[cameraValues.saveIndex]=cameraValues.y[index];
  end

	--CPPInterface:MoveCameraXAbs(cameraValues.x[cameraValues.loadIndex]);
	--CPPInterface:MoveCameraYAbs(cameraValues.y[cameraValues.saveIndex]); --don't delay, get latest value

	cameraValues.loadIndex = cameraValues.loadIndex + 1;
	cameraValues.saveIndex = cameraValues.saveIndex + 1;
	if(cameraValues.loadIndex>=CAMERA_BUFFER)then
		cameraValues.loadIndex=0;
	end
	if(cameraValues.saveIndex>=CAMERA_BUFFER)then
		cameraValues.saveIndex=0;
	end--]]
end

function heor.LockInput(self, frames)
	self.lockTimer=frames;
	self.inputLock=true;
end

function heor.UnlockInput(self)
	self.inputLock=false;
	self.lockTimer=0;
  self.currentState=self.STATE_NORMAL;
end

function heor.UpdateLock(self)
	if(self.lockTimer>0)then
		self.lockTimer= self.lockTimer - 1;
	end
	if(self.lockTimer==0)then
		self:UnlockInput();
    self.lockTimer=-1;
	end
  if(self.roll_timer>0)then
		self.roll_timer = self.roll_timer - 1;
	end
end

function heor.LandOnPlatform(self)
  if(self.collision.groundTouch)then
		if(not self.collision.prevGroundTouch) then
      if( math.abs(self.angleSigned)<25 )then
        if(self.angleSigned>=0)then
          self.groundSpeed = self.xspd;
        else
          self.groundSpeed = self.xspd;
        end
      elseif( math.abs(self.angleSigned)<=45 ) then
        if(math.abs(self.xspd) > math.abs(self.yspd))then
          self.groundSpeed = self.xspd;
        elseif(self.angleSigned>0)then
          self.groundSpeed = self.yspd*-1*0.5*math.abs((math.cos(math.rad(self.angle))));
        else
          self.groundSpeed=self.yspd*0.5*math.abs((math.cos(math.rad(self.angle))));
        end
			
      else
        if(self.angleSigned>0)then
          self.groundSpeed = self.yspd*-1--*math.abs((math.sin(math.rad(angle))));
        else
          self.groundSpeed = self.yspd--*math.abs((math.sin(math.rad(angle))));
        end  
      end
      self.yspd=0;
		end
  end
end

function heor.NewRopeSwing(self, maxTime, distanceFromCenter, direction)
  local CalculateY= function (x, dis) return ((x^2)/(4*dis))*-1; end
  local time=0;
  local moveDirection= direction or self.facingDir;
  
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
    
    self.mySpriteComp:SetRotation(self.mySpriteID, -75*moveDirection);
    --CPPInterface:WriteError(tostring(math.sqrt( (x^2) * (y^2) ) / distanceFromCenter));
    
    --return movement vector
    return CPP.Coord2df(x-xPrevious, y-yPrevious);
  end
end

function heor.ApplySlopeFactor(self)
  if(  ( (self.angle>30)and(self.angle<180) ) or ( (self.angle<330)and(self.angle>180) )  )then
    local extraSpeed = self.SLOPE_GRAVITY * math.sin(math.rad(self.angle))*-1;
    
    local zeroGSPD=(self.groundSpeed==0);
    local positiveGSPD=(self.groundSpeed>0);
    local newGSPD;	
    self.groundSpeed = self.groundSpeed + extraSpeed;
    newGSPD=(self.groundSpeed>0);
    if( (positiveGSPD~=newGSPD) and (not zeroGSPD) )then
      --lock input if direction changes
      --LockInput(30);
    end
  end
end

function heor.ApplyFrictionGravity(self)
  local friction; --Friction value used for this frame
  local gravityFrame; --Gravity for this frame;
  if(self.currentState==self.STATE_WALLSLIDE)then
    gravityFrame=self.WALLJUMP_GRAVITY;
  else
    gravityFrame=self.GRAVITY;
  end
	friction= self.FRICTION_MODIFER * self.tileFriction
  
  --GRAVITY
  if (not self.collision.groundTouch) then
    self.yspd = self.yspd+gravityFrame;
    self.mySpriteComp:SetRotation(self.mySpriteID, 0);
    self.angle=0;
    if(self.currentState==self.STATE_WALLSLIDE)then
      if(self.yspd > self.WALLJUMP_YSPD_MAX)then
        self.yspd = self.WALLJUMP_YSPD_MAX;
      end
    end
  end
  
  if ((not self.input.key.inputL) and (not self.input.key.inputR) and (self.collision.groundTouch))then
    friction=friction*5;
  end
  if ( ((not self.input.key.inputL) and (not self.input.key.inputR)) or (self.inputLock) )then
    --IF TOUCHING THE GROUND
		if(self.collision.groundTouch)then
			if(self.groundSpeed>0)then
				if(friction>=self.groundSpeed)then
					self.groundSpeed=0;
				else
					self.groundSpeed = self.groundSpeed - friction;
				end
		
			elseif(self.groundSpeed<0)then
				if(self.groundSpeed>=(friction*-1) )then
					self.groundSpeed=0;
				else
					self.groundSpeed = self.groundSpeed + friction;
				end
			end
    --IF NOT TOUCHING THE GROUND
		else
			if(self.xspd>0)then
				if(self.FRICTION_AIR>=self.xspd)then
					self.xspd = 0;
				else
					self.xspd = self.xspd - self.FRICTION_AIR;
				end
		
			elseif(self.xspd < 0)then
				if(self.xspd >= (self.FRICTION_AIR*-1))then
					self.xspd = 0;
				else
					self.xspd= self.xspd + self.FRICTION_AIR;
				end
			end
		end
	end

end

function heor.MovementUpdatePipeline(self)
  self:LandOnPlatform()
  self:ApplySlopeFactor()
  self:ApplyFrictionGravity()
end

function heor.InputJump(self)
    if self.collision.tileCollision.previous.tileUp == true then return; end
    self.CPPInterface:PlaySound(self.SoundJump);
    if(math.abs(self.angleSigned)<25)then
      self.yspd = self.JUMPHEIGHT;
    else
      local yCos=math.abs(math.cos(math.rad(self.angle)));
      local xSin=math.sin(math.rad(self.angle));
      self.yspd = (self.JUMPHEIGHT * yCos)
      self.xspd = (self.JUMPHEIGHT * xSin)
    end
  
		self.groundSpeed=0;
    self.angle=0;
		self.collision.groundTouch=false;
    if(self.currentState==self.STATE_ROLL)then --long jump
      self.xspd = self.xspd * 1.5;
    end
    
    self.CPPInterface:EventLuaBroadcastEvent(self.EID, "JUMP");
		self:UnlockInput();
end
function heor.InputWallJump(self)
    self.CPPInterface:PlaySound(self.SoundJump);
    self.xspd = self.WALLJUMP_XSPD * self.facingDir;
    self.yspd = -self.WALLJUMP_YSPD;

    self:ChangeState(self.STATE_NORMAL);
		self:LockInput(self.WALLJUMP_LOCK);
end
  
function heor.InputHorizontal(self, direction)
  local absGS=math.abs(self.groundSpeed);
  local absX= math.abs(self.xspd);
  local movDir= (self.groundSpeed>=0);
  if(movDir==false)then
    movDir=-1;
  else
    movDir=1;
  end
  
  --UPDATE GSPD (GROUND)
  if(self.collision.groundTouch) then
    if(movDir==direction) then --Add friction to the ground speed (slowing him faster) if moving against momentum
      if(absGS<self.ACCELERATION_TOP)then
        self.groundSpeed= self.groundSpeed + (self.ACCELERATION * direction);
      end
    else
      self.groundSpeed= self.groundSpeed + (self.DEACCELERATION * direction);
		end
  end
  
  --UPDATE XSPD (AIR)
  if(not self.collision.groundTouch) then
    movDir= (self.xspd>=0);
    if(movDir==false)then
      movDir=-1;
    else
      movDir=1;
    end
    if(movDir==direction) then
      if(absX<self.ACCELERATION_TOP)then
        self.xspd= self.xspd + (self.ACCELERATION_AIR * direction);
      end
    else
			self.xspd= self.xspd + ((self.ACCELERATION_AIR + self.FRICTION_AIR) * direction);
    end
  end
end
function heor.AnimateNormal(self)
  local newImgSpd=math.abs(self.groundSpeed)/16;
	if(newImgSpd>2)then
		newImgSpd=2;
	end

	if(self.groundSpeed > self.ACCELERATION)then
		self.mySpriteComp:SetAnimation     (self.mySpriteID, "Walk");
		self.mySpriteComp:SetAnimationSpeed(self.mySpriteID, newImgSpd);
    self.facingDir = self.FACING_RIGHT;
		
	elseif(self.groundSpeed < -(self.ACCELERATION))then  
		self.mySpriteComp:SetAnimation     (self.mySpriteID, "Walk");
		self.mySpriteComp:SetAnimationSpeed(self.mySpriteID, newImgSpd);  
    self.facingDir = self.FACING_LEFT;
		
	elseif(self.groundSpeed==0)then  
		self.mySpriteComp:SetAnimation     (self.mySpriteID,"Stand");
    self.mySpriteComp:SetAnimationSpeed(self.mySpriteID, 0);
	end
  
  self.mySpriteComp:SetScalingX(self.mySpriteID, self.facingDir);
  
  self.mySpriteComp:SetScalingY(self.mySpriteID, 1);
  
  
  if(self.collision.groundTouch)then
    self.mySpriteComp:SetRotation(self.mySpriteID, -self.angle);
    --mySpriteComp:SetRotation(mySpriteID, -angle-rotationMode.mAngle);
  else
    self.mySpriteComp:SetRotation(self.mySpriteID, 0);
  end
  
end

function heor.AnimateOther(self)
  self.mySpriteComp:SetAnimation     (self.mySpriteID, "Stand");
  self.mySpriteComp:SetAnimationSpeed(self.mySpriteID, 0);
  
  self.mySpriteComp:SetScaling       (self.mySpriteID, self.facingDir, 1);

  if(self.collision.groundTouch)then
    self.mySpriteComp:SetRotation(self.mySpriteID, -self.angle);
  else
    self.mySpriteComp:SetRotation(self.mySpriteID, 0);
  end
end

function heor.AnimateRoll(self)
  self.mySpriteComp:SetScalingY(self.mySpriteID, 1);
  self.mySpriteComp:SetRotation(self.mySpriteID, (360*(self.lockTimer/self.ROLL_TIMER)) * -1 * self.facingDir);
end

function heor.UpdateInputs(self)
  for k,v in pairs(self.input.key) do
    if      ((v==true)and(self.input.keyPrevious[k]==false))then
      self.input.keyPress[k]=true;
      self.input.keyRelease[k]=false;
    elseif  ((v==false)and(self.input.keyPrevious[k]==true))then
      self.input.keyPress[k]=false;
      self.input.keyRelease[k]=true;
    else
      self.input.keyPress[k]=false;
      self.input.keyRelease[k]=false;
    end
    self.input.keyPrevious[k]=v;
  end
end


function heor.ChangeState(self, newState)
  self.externalUseWhipInterrupt=true;
  if(newState == self.STATE_NORMAL) then
    self:UnlockInput();
    
  elseif(newState == self.STATE_ROLL) then
    self:LockInput(self.ROLL_TIMER);
    
  elseif(newState==self.STATE_WALLSLIDE) then
    self.xspd=0;
    if(self.yspd<0)then
      self.yspd=0;
    end
    
  end
  if (newState==nil)then
    assert(nil, "NEWSTATE IS NIL")
    
    self.CPPInterface:WriteError(self.EID, "NIL");
  end
  self.currentState=newState;
end

function heor.Update(self)
  self.externalUseWhipInterrupt=false;
  self.externalUseWhip=false;
  if(self.ropeSwing~=nil)then    --if swinging, then just execute this block of code and return
    local returnVal=self:ropeSwing();
    self.platformVelocityX=0;
    self.platformVelocityY=0;
    self.myPositionComp:ChangeParent(0);
    if(returnVal~=nil)then
      self.myPositionComp:SetMovement(returnVal);
      self.xspd=returnVal.x;
      self.yspd=-3.5;
      return;
    else
      self.ropeSwing=nil;
    end
  end
  
  
  if((self.collision.groundTouch==true)and(self.currentState == self.STATE_WALLSLIDE))then
    self:ChangeState(self.STATE_NORMAL);
  end
  
  --if you land on the ground, turn off the attack lock
  if((self.collision.groundTouch==true)and(self.attackLock==true))then
    self:UnlockInput();
    self.attackLock=false;
  end
  
  
  self:UpdateInputs();
  self:MovementUpdatePipeline();
  Vec2d = self.myPositionComp:GetPositionWorld();
  --MoveCamera(Vec2d.x, Vec2d.y);
  self.angleSigned = AngleToSignedAngle(self.angle);
  
	
	----------------
	--Handle Input--
	----------------
  if((self.input.keyPress.inputSwing) and (not self.inputLock))then
    self.ropeSwing=self:NewRopeSwing(18, 75); --these parameters are perfect :D
    self.xspd=0;
    self.yspd=0;
    self.groundSpeed=0;
    self.platformVelocityX=0;
    self.platformVelocityY=0;
    self.myPositionComp:ChangeParent(0);
  end
  
  if((self.input.keyPress.inputWhip) and (not self.inputLock))then
    self.externalUseWhip=true;
  end
  
  if((self.input.key.inputThrow) and (not self.inputLock))then
    if(self.groundSpeed<0)then
      self.groundSpeed = -self.MAXSPEED;
    else
      self.groundSpeed = self.MAXSPEED;
    end
  end
	
	if (self.input.keyPress.inputU) then
    if(self.currentState == self.STATE_WALLSLIDE)then
      self:InputWallJump();
    elseif (self.collision.groundTouch) then
      self:InputJump();
    end
	end
  if ( (self.input.keyRelease.inputU) and (not self.collision.groundTouch) ) then
    if(self.yspd<(-2))then
			self.yspd=(-2);
		end
  end
	if ( (self.input.key.inputR) and (not self.inputLock) ) then
    if((self.collision.tileCollision.previous.tileRight==true)and(not self.collision.groundTouch)and(self.yspd>=0))then
      self.facingDir = self.FACING_LEFT; --face opposite way of wall slide
      self:ChangeState(self.STATE_WALLSLIDE);
    else
      self:InputHorizontal(self.FACING_RIGHT);
    end
	end
  if ( (self.input.keyRelease.inputR) and (self.facingDir == self.FACING_LEFT) ) then
    if(self.currentState == self.STATE_WALLSLIDE)then
      self:ChangeState(self.STATE_NORMAL);
    end
  end
	if ( (self.input.key.inputL) and (not self.inputLock) ) then
    if((self.collision.tileCollision.previous.tileLeft==true) and(not self.collision.groundTouch)and(self.yspd>=0))then
      self.facingDir = self.FACING_RIGHT; --face opposite way of wall slide
      self:ChangeState(self.STATE_WALLSLIDE);
    else
      self:InputHorizontal(self.FACING_LEFT);
    end
	end
  if ( (self.input.keyRelease.inputL) and (self.facingDir==self.FACING_RIGHT) ) then
    if(self.currentState==self.STATE_WALLSLIDE)then
      self:ChangeState(self.STATE_NORMAL);
    end
  end
  
  if ( (self.input.key.inputD) and (not self.inputLock) ) then
    if((self.currentState==self.STATE_NORMAL)and(self.collision.groundTouch==true)and(self.roll_timer==0))then
      self:ChangeState(self.STATE_ROLL);
      self.roll_timer = self.ROLL_COOLDOWN;
    end
	end
	
  -----------
	--Animate--
	-----------
	if    (self.currentState == self.STATE_ROLL)  then
    self:AnimateRoll();
    --self.CPPInterface:WriteError(self.EID, "ROLL");
    
  elseif(self.currentState == self.STATE_NORMAL)then
    self:AnimateNormal();
    
    --self.CPPInterface:WriteError(self.EID, "NORMAL");
    
  else
    self:AnimateOther();
    --self.CPPInterface:WriteError(self.EID, tostring(self.currentState));
  end
  
	----------------------
	--Set X and Y Speeds--
	----------------------
	local speedDampenX=1;
  local speedDampenY=1;
  
  if(self.currentState==self.STATE_ROLL)then
    self.groundSpeed = self.ROLL_SPEED * self.facingDir;
  end
  
	if(self.groundSpeed > self.MAXSPEED) then
		self.groundSpeed = self.MAXSPEED;
	elseif(self.groundSpeed < (-self.MAXSPEED)) then
		self.groundSpeed=-self.MAXSPEED;
	end
  
  if(self.xspd > self.MAXSPEED) then
		self.xspd = self.MAXSPEED;
	elseif(self.xspd < (-self.MAXSPEED)) then
		self.xspd = -self.MAXSPEED;
	end
  
  if(self.yspd > self.MAXSPEED) then
		self.yspd = self.MAXSPEED;
	elseif(self.yspd < (-self.MAXSPEED)) then
		self.yspd = -self.MAXSPEED;
	end
  
  if(self.collision.groundTouch) then
    self.xspd = self.groundSpeed;
    self.yspd = 0;
    speedDampenX=math.cos(math.rad(self.angle));
  else
    speedDampenX=1;
    speedDampenY=1;
  end
  
  if(self.collision.groundTouch==false)then
    self.xspd = self.xspd + self.platformVelocityX;
    self.yspd = self.yspd + self.platformVelocityY;
    self.myPositionComp:ChangeParent(self.parent);
    platformVelocityX=0;
    platformVelocityY=0;
  end
  
  -------------------
	--Update CPP Data--
	-------------------
  --
  updateVec=CPP.Coord2df((self.xspd*speedDampenX), (self.yspd*speedDampenY));
  self.cameraValues.rect.x=-16 - (self.xspd*2);
  
	self.myPositionComp:SetMovement(updateVec);
	
	-----------------------------------------
	--Prepare for TCollision and Next Frame--
	-----------------------------------------
  
  self:UpdateLock();
  
  Vec2d = self.myPositionComp:GetPositionWorld();
  local xx=Vec2d.x;
  local yy=Vec2d.y;
  
  if(self.currentState==self.STATE_WALLSLIDE)then
    if((self.collision.tileCollision.previous.tileRight==false)and(self.facingDir==self.FACING_LEFT))
    or((self.collision.tileCollision.previous.tileLeft==false) and(self.facingDir==self.FACING_RIGHT))then
      self:ChangeState(self.STATE_NORMAL);
    end
  end
  
  self.prevGroundSpeed = self.groundSpeed;
  self.collision.Update(self.xspd, self.yspd);
end

function heor.LandOnGround(self, ycoordinate, angleGround)
  --Update position
    newPosition= CPP.Coord2df(0,ycoordinate);
    
    newPosition= self.myPositionComp:TranslateWorldToLocal(newPosition);
    
    self.myPositionComp:SetPositionLocalY(newPosition.y);
    
    --Update variables
    self.collision.groundTouch=true;
		self.angle=angleGround;
end

function heor.OnEntityCollision(self, entityID, packet)
  --Need to get height and collision type
  local entHeight=16;
  local myBoxID=packet:GetID();
  local objectType=packet:GetType();
  local objectName=packet:GetName();

  if(objectType==self.LEngineData.objTypes[1])then --Exit
    if(myBoxID==self.collision.entityCollision.boxID.CBOX_PRIME)then
      if((self.input.key.inputThrow) and (not self.inputLock))then
        self.CPPInterface:EventEngineMapChange(packet);
      end
    end

  elseif((myBoxID==self.collision.entityCollision.boxID.CBOX_PRIME)and(objectType=="PREFAB_COIN"))then
    self.coinCount= coinCount + 1;
    self.coinText:ChangeText("Coins : " .. tostring(coinCount));

  elseif ((myBoxID==self.collision.entityCollision.boxID.CBOX_ENT_DOWN))then
  --How do i check if object is solid?
  --[[local solid = self.CPPInterface:EntityGetProperty(entityID,"LEngine.InitializationTable.Solid");
    if(solid==true)then
      if(self.yspd>=0)then
        self.collision.groundTouch=true;
        self:LandOnGround(self.CPPInterface:GetEntityPositionWorld(entityID).y-entHeight, 0);
        self.myPositionComp:ChangeParent(entityID);
      
        local vecMove = self.CPPInterface:GetEntityMovement(entityID);
        self.platformVelocityX=vecMove.x;
        self.platformVelocityY=vecMove.y;
      
        self.angle=0;
      end
    end]]--
  end
end
  

function heor.OnTileCollision(self, packet)
  local absoluteCoords=self.myPositionComp:GetPositionWorldInt();
  self.collision.OnTileCollision(self, packet, self.xspd+self.platformVelocityX, self.yspd+self.platformVelocityY, absoluteCoords.x, absoluteCoords.y);
end

function heor.OnTileDown(self, newPosition, newAngle)
  --Update position
  local absoluteCoords=self.myPositionComp:GetPositionWorld();
  newPosition= self.myPositionComp:TranslateWorldToLocal(newPosition);
  --newPosition.y=absoluteCoords.y-newPosition.y;
  --newPosition.x=0;
  self.myPositionComp:SetPositionLocalY(newPosition.y);
    
  --Update variables
  self.angle=newAngle;
  self.debug_tcolx=tx;
  self.debug_tcoly=ty;
  self.myPositionComp:ChangeParent(self.parent);
  self.platformVelocityX=0;
  self.platformVelocityY=0;
end

function heor.OnTileRight(self, newPosition)
  newPosition= self.myPositionComp:TranslateWorldToLocal(newPosition);
  self.myPositionComp:SetPositionLocalX(newPosition.x);
  self.xspd=0; --for when in the air
  self.groundSpeed=0;
  self.mySpriteComp:SetAnimationSpeed (self.mySpriteID, 0);
  self.mySpriteComp:SetAnimation      (self.mySpriteID, "Stand");
end

function heor.OnTileLeft(self, newPosition)
  newPosition= self.myPositionComp:TranslateWorldToLocal(newPosition);
  self.myPositionComp:SetPositionLocalX(newPosition.x);
  self.xspd=0; --for when in the air
  self.groundSpeed=0;
  self.mySpriteComp:SetAnimationSpeed (self.mySpriteID, 0);
  self.mySpriteComp:SetAnimation      (self.mySpriteID, "Stand");
end

function heor.OnTileUp(self, newPosition)
  if (self.yspd<0) then
    self.myPositionComp:TranslateWorldToLocal(newPosition);
    self.myPositionComp:SetPositionLocalY(newPosition.y);
    self.yspd=0;
  end
  self.mySpriteComp:SetAnimationSpeed(self.mySpriteID, 0);
end

function heor.TakeDamage(self, hitpoints, attackerEID)
  self.xspd = self.KNOCKBACK_SPEED_X * self.facingDir*-1;
  self.yspd = self.KNOCKBACK_SPEED_Y;
  self.attackLock=true;
  self:LockInput(30);
  
  self.health = self.health-hitpoints;
  HUD.text.health:ChangeText("Health : " .. tostring(health));
      
  self.CPPInterface:EventLuaSendEvent(attackerEID, "HURT");--Let the attacking object know that this one was hurt
end

function heor.OnLuaEvent(self, senderEID, eventString)
  if(eventString=="ATTACK")then --if being attacked
    --[[if(self.currentState ~= self.STATE_ROLL)then--if not rolling
      self:TakeDamage(1,senderEID);
    end]]--
  end
end

return heor;
end

return NewHeor();