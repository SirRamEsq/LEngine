function NewFallingPlatform(baseclass)
      local platform = baseclass or {}

      platform.c = {}
      platform.c.WIDTH  = 32
      platform.c.HEIGHT = 16
      platform.c.ALARM  = 1
      platform.c.ALARM_OFFSET  = 2
      platform.c.OFFSET_TIME = 2 + math.floor(math.random(5))

      platform.c.FALL_TIME  = 60
      platform.c.FALL_SPEED = 4
      platform.c.GRAVITY    = 0.21875;
      platform.c.MAXSPEED   = 8

      platform.c.DIR_UP   = 0
      platform.c.DIR_DOWN = 1

      platform.falling = false

      platform.respawnY = 30000
      platform.initialCoordinates = {}
      platform.initialCoordinates.x = 0
      platform.initialCoordinates.y = 0

      platform.xspd = 0
      platform.yspd = 0

      platform.spriteOffset     = {}
      platform.spriteOffset.xDefault   = (platform.c.WIDTH/2)
      platform.spriteOffset.yDefault   = (platform.c.HEIGHT/2)
      platform.spriteOffset.x   = platform.spriteOffset.xDefault
      platform.spriteOffset.y   = platform.spriteOffset.yDefault
      platform.spriteOffset.DIR = platform.c.DIR_UP
      platform.spriteOffset.max = platform.spriteOffset.yDefault + 2
      platform.spriteOffset.min = platform.spriteOffset.yDefault - 2

      local result=0;
      --security hole here, user can just use '..' to go wherever they want
      result, platform.collision = pcall(loadfile(commonPath .. "/collisionSystem.lua", _ENV))
      result, platform.timing    = pcall(loadfile(commonPath .. "/timing.lua", _ENV))

      function platform.Initialize()
        -----------------------
        --C++ Interface setup--
        -----------------------

        platform.depth        = platform.LEngineData.depth;
        platform.parent       = platform.LEngineData.parent;
        platform.CPPInterface = CPP.interface
        platform.EID          = platform.LEngineData.entityID;

        platform.CompSprite     = platform.CPPInterface:GetSpriteComponent    (platform.EID);
        platform.CompCollision  = platform.CPPInterface:GetCollisionComponent (platform.EID);
        platform.CompPosition   = platform.CPPInterface:GetPositionComponent  (platform.EID);

        ----------------
        --Sprite setup--
        ----------------
        platform.sprite = platform.CPPInterface:LoadSprite("SpritePlatform.xml");
        if(platform.sprite==nil) then
          platform.CPPInterface:WriteError(platform.EID, "sprite is NIL");
        end

        --Logical origin is as at the top left; (0,0) is top left
        --Renderable origin is at center;       (-width/2, -width/2) is top left
        --To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
        platform.spriteID = platform.CompSprite:AddSprite(platform.sprite, platform.depth, platform.spriteOffset.xDefault, platform.spriteOffset.yDefault);
        platform.CompSprite:SetAnimation(platform.spriteID, "Stand");
        platform.CompSprite:SetRotation(platform.spriteID, 0);

        platform.CompCollision:SetName(platform.LEngineData.name)
        platform.CompCollision:SetType(platform.LEngineData.objType);

        platform.collision.Init(platform.c.WIDTH, platform.c.HEIGHT, platform.CPPInterface, platform.CompCollision, platform.EID);

        platform.timing:SetAlarm(platform.c.ALARM, platform.c.FALL_TIME, platform.OnFall, false) --don't repeat alarms
        platform.timing:SetAlarm(platform.c.ALARM_OFFSET, platform.c.OFFSET_TIME, platform.UpdateSpriteOffset, true) --repeat
        platform.timing:GetAlarm(platform.c.ALARM):Disable();

        platform.movement= CPP.Coord2df(0, 0)

        local worldPos = platform.CompPosition:GetPositionWorld()
        platform.initialCoordinates.x = worldPos.x
        platform.initialCoordinates.y = worldPos.y

        platform.CompPosition:SetMaxSpeed(platform.c.MAXSPEED)
      end

      function platform.UpdateSpriteOffset()
        if(platform.spriteOffset.DIR == platform.c.DIR_UP)then
          platform.spriteOffset.y = platform.spriteOffset.y + 1
          if(platform.spriteOffset.y >= platform.spriteOffset.max)then
            platform.spriteOffset.DIR = platform.c.DIR_DOWN
          end
        else
          platform.spriteOffset.y = platform.spriteOffset.y - 1
          if(platform.spriteOffset.y <= platform.spriteOffset.min)then
            platform.spriteOffset.DIR = platform.c.DIR_UP
          end
        end

        platform.CompSprite:SetOffset(platform.spriteID, platform.spriteOffset.xDefault, platform.spriteOffset.y);
      end

      function platform.Update()
        if(platform.CompPosition:GetPositionWorld().y > platform.respawnY)then
          platform.Respawn()
        end

        --platform.movement= CPP.Coord2df(platform.xspd, platform.yspd)
        --platform.CompPosition:SetMovement(platform.movement);
        platform.movement = platform.CompPosition:GetMovement();
        platform.collision.Update(platform.movement.x, platform.movement.y);

        platform.timing:Update();
      end

      function platform.OnEntityCollision(entityID, packet)

      end

      function platform.Respawn()
        platform.CompPosition:SetPositionLocalY(platform.initialCoordinates.y)
        platform.CompPosition:SetAccelerationY(0)
        platform.CompPosition:SetMovementY(0)
        platform.falling = false
        platform.timing:GetAlarm(platform.c.ALARM_OFFSET):Restart();
      end

      function platform.Land()
        if (platform.falling == false)then
          platform.falling = true;
          platform.CompSprite:SetOffset(platform.spriteID, platform.spriteOffset.xDefault, platform.spriteOffset.yDefault);
          platform.timing:GetAlarm(platform.c.ALARM):Restart()
          platform.timing:GetAlarm(platform.c.ALARM_OFFSET):Disable();
        end
      end

      function platform.OnFall()
        platform.respawnY = platform.CPPInterface:GetMap():GetHeightPixels() + platform.initialCoordinates.y
        platform.CompPosition:SetAccelerationY(platform.c.GRAVITY)
      end

      function platform.OnLuaEvent(senderEID, eventString)

      end


      function platform.OnTileCollision(senderEID, eventString)

      end


      platform.EntityInterface            = platform.EntityInterface or {}
      platform.EntityInterface.IsSolid    = function ()       return true; end
      platform.EntityInterface.Land       = platform.Land

      return platform;
end

return NewFallingPlatform;
