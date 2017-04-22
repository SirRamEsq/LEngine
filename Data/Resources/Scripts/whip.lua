--Initialization Parameters:
--    WHIP_HOLDER_EID: EID of the entity holding this whip

--DEBUG
package.path = package.path .. ";/opt/zbstudio/lualibs/mobdebug/?.lua"
--require('mobdebug').start()

--Constants
	ANIMATION_WIDTH=82;
  ANIMATION_HEIGHT=48;
  
  WHIP_HOLDER_EID=nil;
  previousUseWhip=false;
  useWhip=false;
  useWhipInterrupt=false;
  whipAttack=nil;
  whipAttackCurrentFrame=0
  
  MAX_FRAMES=6;
  
  SPRITE_NAME="SPRITE_Whip"
  
  animationSpeed=.25;
  
  --C++ Interfacing
		CPPInterface=nil;
		mySprite=nil;
		mySpriteComp=nil;
		myColComp=nil;
    myPositionComp=nil;
		EID=0;
		depth=0;
		parent=0;
    
    
function Initialize()
	--------------------
	--LUA Verion Check--
	--------------------
  
	if (_VERSION == "Lua 5.1") then
		LUA51=true;
	else
		LUA51=false;
	end
  
  collision={}
  collision.boxes={}
  collision.boxes.primary={}
  collision.boxes.bottomleft={}
  collision.boxes.top={}
  collision.boxes.topright={}
  collision.boxes.right={}
  
    collision.boxes.primary.x =  0
    collision.boxes.primary.y =  0
    collision.boxes.primary.w = 83
    collision.boxes.primary.h = 49
    collision.boxes.primary.priority = 10
    collision.boxes.primary.id = 1
    
    collision.boxes.bottomleft.x =  0
    collision.boxes.bottomleft.y = 26
    collision.boxes.bottomleft.w = 21
    collision.boxes.bottomleft.h = 15
    collision.boxes.bottomleft.priority = 10
    collision.boxes.bottomleft.id = 2
    
    collision.boxes.top.x = 24
    collision.boxes.top.y =  0
    collision.boxes.top.w = 20
    collision.boxes.top.h = 40
    collision.boxes.top.priority = 10
    collision.boxes.top.id = 3
    
    collision.boxes.topright.x = 44
    collision.boxes.topright.y = 16
    collision.boxes.topright.w = 31
    collision.boxes.topright.h = 19
    collision.boxes.topright.priority = 10
    collision.boxes.topright.id = 4
    
    collision.boxes.right.x = 35
    collision.boxes.right.y = 35
    collision.boxes.right.w = 48
    collision.boxes.right.h = 8
    collision.boxes.right.priority = 10
    collision.boxes.right.id = 5
  
	-----------------------
	--C++ Interface setup--
	-----------------------
  
	
	depth=LEngine.engineProperties.depth;
	parent=LEngine.engineProperties.parent;
	CPPInterface=LEngine.CPPInterface;
	EID=LEngine.engineProperties.entityID;

  
  LEngine.name= LEngine.name or "Whip"
  
	CPPInterface:AddSpriteComponent();
  CPPInterface:AddCollisionComponent();
	mySpriteComp=CPPInterface:GetSpriteComponent();
  myColComp=CPPInterface:GetCollisionComponent();
  myPositionComp=CPPInterface:GetPositionComponent();
	----------------
	--Sprite setup--
	----------------
	
	mySpriteComp:SetRotation(0);
	mySprite=CPPInterface:LoadSprite(SPRITE_NAME);
  ANIMATION_NAME="ANIMATION_WHIP";
	if(mySprite==nil) then
		CPPInterface:WriteError("sprite is NIL");
	else
		mySprite:AddAnimation(ANIMATION_NAME);

		aniRect=CRect(0,0,ANIMATION_WIDTH,ANIMATION_HEIGHT);
	  
    for i=0, 6 do
      mySprite:AppendImage(ANIMATION_NAME,aniRect,"GrassTerrain.png");
      aniRect.x= aniRect.x + ANIMATION_WIDTH
    end
    
		mySpriteComp:AddSprite(mySprite);
		mySpriteComp:SetCurrentSprite(mySprite:GetName());
		mySprite=mySpriteComp:GetCurrentSprite();
		mySprite:SetSpeed(animationSpeed);
		mySprite:SetColorKey(ANIMATION_NAME,0,255,0,255);
		mySprite:SetCurrentAnimation(ANIMATION_NAME);
	end
	
	mySpriteComp:SetDepth(depth);
  myColComp:SetName(LEngine.engineProperties.name)
  myColComp:SetType(LEngine.engineProperties.objType);
  
  
  for k,v in pairs(collision.boxes) do
    rectangle=CRect(v.x, v.y, v.w, v.h);
	
    myColComp:AddCollisionBox(rectangle, v.id, 0);
    myColComp:CheckForEntities(v.id);
  end
  
  myColComp:SetPrimaryCollisionBox(collision.boxes.primary.id, false);
  --Initialization Parameters:
--    direction ["left", "right"]:  Direction that the arrow moves in
  dir=LEngine.InitializationTable.direction or "right"
  if dir == "left" then DIRECTION=DIRECTION_LEFT else DIRECTION=DIRECTION_RIGHT end
  
  WHIP_HOLDER_EID=LEngine.InitializationTable.WHIP_HOLDER_EID;
  if(WHIP_HOLDER_EID==nil)then
    CPPInterface:EntityDelete(EID);
  else
    CPPInterface:EntityLinkExternalData(WHIP_HOLDER_EID,"externalUseWhip");
    CPPInterface:EntityLinkExternalData(WHIP_HOLDER_EID,"externalUseWhipInterrupt");
    CPPInterface:EventLuaObserveEntity(WHIP_HOLDER_EID);
  end
  
  
end

function NewWhipAttack()
  whipAttackCurrentFrame=0;
  mySpriteComp:SetRender(true);
  mySprite:FirstImage();
  mySprite:SetSpeed(animationSpeed);
      
  return function()
    if(whipAttackCurrentFrame>MAX_FRAMES)then
      mySpriteComp:SetRender(false);
      mySprite:SetSpeed(0);
      return false;
    end
    
    whipAttackCurrentFrame= whipAttackCurrentFrame + animationSpeed;
    return true
  end
end

--[[
PROBLEM
DECLARED VARS DONT APEPAR 
]]--

function Update()
  useWhip=true;--LEngine.ExternalData.externalUseWhip.GetData:ToBool()
  useWhipInterrupt=false;--LEngine.ExternalData.externalUseWhipInterrupt.GetData:ToBool()
  
  if(whipAttack==nil)then
    if(useWhip==true and useWhipInterrupt==false)then
      whipAttack=NewWhipAttack();
    end
    
  else
    if(useWhipInterrupt==true)then
      whipAttack=nil;
      
    elseif(whipAttack()==false)then
      whipAttack=nil;
    
    end
  end
  
  previousUseWhip=useWhip
  
end

function OnEntityCollision(entityID, packet)
  local boxID=packet:GetID();
  local name= packet:GetName();
  local ttype= packet:GetType();
  local attack=false;
  globalCol=globalCol+1;
  
  if(boxID~=collision.boxes.primary.id)then --if box isn't primary
    if(boxID==collision.boxes.bottomleft.id   and whipAttackCurrentFrame>=1 and whipAttackCurrentFrame<=3)then --1-3
      attack=true;
    elseif(boxID==collision.boxes.top.id      and whipAttackCurrentFrame>=3 and whipAttackCurrentFrame<=5)then--3-5
      attack=true;
    elseif(boxID==collision.boxes.topright.id and whipAttackCurrentFrame>=5 and whipAttackCurrentFrame<=7)then--5-7
      attack=true;
    elseif(boxID==collision.boxes.right.id    and whipAttackCurrentFrame>=5 and whipAttackCurrentFrame<=7)then--5-7
      attack=true;
    end
  end
  
  if(attack)then
    CPPInterface:EventLuaSendEvent(entityID, "ATTACK"); --may result in recieving a "hurt" message, thus confirming an attack on the entity
  end
end

function OnTileCollision(packet)
  
end


function OnLuaEvent(senderEID, eventString)
  if(eventString=="[ENTITY_DELETED]")then --message from the engine
    if(senderEID==WHIP_HOLDER_EID)then--This shared data has been deleted
      LEngine.ExternalData.externalUseWhip=nil;
      LEngine.ExternalData.externalUseWhipInterrupt=nil;
    end
  end
end