function NewRemoveTiles(baseclass)
      local entity = baseclass or {}

      entity.c = {}
      entity.c.STATE_ON = true
      entity.c.STATE_OFF = false

      entity.defaultState = entity.c.STATE_OFF
      entity.defaultToggleOnce = false
      entity.defaultSpriteName = "SpriteButton.xml"

      entity.WIDTH=16
      entity.HEIGHT=16

      entity.currentlyToggled = false
      entity.currentlyToggledPrevious = false

      entity.toggled = false

      entity.erase = {}

      entity.spriteOffset     = {}
      entity.spriteOffset.xDefault   = entity.WIDTH/2
      entity.spriteOffset.yDefault   = entity.HEIGHT/2

      entity.tilesToRemoveArea = {}
      entity.tileLayerName = ""
      --2dArray containing removed tile values
      entity.tilesRestore = {}
      entity.map = nil
      entity.tileLayer = nil

      entity.cBoxID=1


      function entity.Initialize()
        -----------------------
        --C++ Interface setup--
        -----------------------

        entity.depth          = entity.LEngineData.depth;
        entity.parent         = entity.LEngineData.parent;
        entity.CPPInterface   = entity.LEngineData.interface;
        entity.EID            = entity.LEngineData.entityID;

        entity.state          = entity.LEngineData.InitializationTable.defaultState or entity.defaultState
        entity.toggleOnce     = entity.LEngineData.InitializationTable.toggleOnce or entity.defaultToggleOnce
        entity.spriteName     = entity.LEngineData.InitializationTable.spriteName or entity.defaultSpriteName
        entity.tileLayerName  = entity.LEngineData.InitializationTable.tileLayerName or ""

        entity.state = false

        entity.tilesToRemoveArea.x = entity.LEngineData.InitializationTable.tilesX
        entity.tilesToRemoveArea.y = entity.LEngineData.InitializationTable.tilesY
        entity.tilesToRemoveArea.w = entity.LEngineData.InitializationTable.tilesW or 0
        entity.tilesToRemoveArea.h = entity.LEngineData.InitializationTable.tilesH or 0

        entity.CompSprite     = entity.CPPInterface:GetSpriteComponent    (entity.EID);
        entity.CompCollision  = entity.CPPInterface:GetCollisionComponent (entity.EID);
        entity.CompPosition   = entity.CPPInterface:GetPositionComponent  (entity.EID);

        ----------------
        --Sprite setup--
        ----------------
        entity.sprite = entity.CPPInterface:LoadSprite(entity.spriteName);
        if(entity.sprite==nil) then
          entity.CPPInterface:WriteError(entity.EID, "sprite is NIL");
        end

        --Logical origin is as at the top left; (0,0) is top left
        --Renderable origin is at center;       (-width/2, -width/2) is top left
        --To consolodate the difference, use the Vec2 offset (WIDTH/2, HEIGHT/2)
        entity.spriteID = entity.CompSprite:AddSprite(entity.sprite, entity.depth, entity.spriteOffset.xDefault, entity.spriteOffset.yDefault);
        entity.CompSprite:SetRotation(entity.spriteID, 0);

        entity.map = entity.CPPInterface:GetMap()
        entity.tileLayer = entity.map:GetTileLayer(entity.tileLayerName)

        entity.CompCollision:SetName(entity.LEngineData.name)
        entity.CompCollision:SetType("")
        entity.CompCollision:AddCollisionBox(CPP.CRect(0,0,entity.WIDTH, entity.HEIGHT), entity.cBoxID, 0)
        entity.CompCollision:CheckForEntities(entity.cBoxID)
        entity.CompCollision:SetPrimaryCollisionBox(entity.cBoxID)

        entity.SetSprite()
      end

      function entity.SetSprite(boolean)
        if(boolean)then
          entity.CompSprite:SetAnimation(entity.spriteID, "On");
        else
          entity.CompSprite:SetAnimation(entity.spriteID, "Off");
        end
      end

      function entity.Update()
        entity.currentlyToggledPrevious = entity.currentlyToggled
        entity.currentlyToggled = false
      end

      function entity.OnLuaEvent(senderEID, eventString)

      end

      function entity.Toggle()
        local tiles = entity.tilesToRemoveArea
        if(entity.state == entity.c.STATE_ON)then
          for ix = tiles.x, tiles.x + tiles.w, 1 do
            for iy = tiles.y, tiles.y + tiles.h, 1 do
              entity.tileLayer:SetTile(ix, iy, entity.tilesRestore[ix][iy])
            end
          end

        elseif(entity.state == entity.c.STATE_OFF)then
          for ix = tiles.x, tiles.x + tiles.w, 1 do
            entity.tilesRestore[ix] = {}
            for iy = tiles.y, tiles.y + tiles.h, 1 do
              entity.tilesRestore[ix][iy] = entity.tileLayer:GetTile(ix,iy)
              entity.tileLayer:SetTile(ix, iy, 0)
              --need to update tile renderer to display changes
            end
          end
        end
        local updateRect = CPP.CRect(tiles.x,tiles.y,tiles.w,tiles.h)
        entity.tileLayer:UpdateRenderArea(updateRect)
        entity.state = not entity.state
        entity.SetSprite(entity.state)
      end

      function entity.Activate()
        if(entity.currentlyToggledPrevious == false)then
          if entity.toggleOnce then
            if entity.toggled == false then
              entity.toggled = true
              entity.Toggle()
            end
          else
            entity.Toggle()
          end
        end
        entity.currentlyToggled = true
      end

      function entity.OnEntityCollision(entityID, packet)

      end


      entity.EntityInterface            = entity.EntityInterface or {}
      entity.EntityInterface.IsSolid    = function ()       return true; end
      entity.EntityInterface.Land       = entity.Activate
      entity.EntityInterface.Atack      = function (damage) entity.Activate(); return 1 end

      return entity;
end

return NewRemoveTiles;
