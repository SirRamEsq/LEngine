--Add Moving Platforms to this
--need to change to use 'self'

--Container name
local collision = {}
collision={};

collision.interface=nil;
collision.collisionComponent=nil;

  --this tile coordinate will be ignored by top and bottom colliders
  --if the coordinate is hit by either the left or right collision boxes
  collision.ignoreTileX=-1;
  collision.WIDTH=0;
  collision.HEIGHT=0;
  collision.TILEWIDTH=16;
  collision.TILEHEIGHT=16;

  collision.tileCollision={}
  collision.tileCollision.footHeightValue=0
  collision.tileCollision.boxID={}
  collision.tileCollision.coordinates={}

  collision.entityCollision={}
  collision.entityCollision.boxID={}
  collision.entityCollision.coordinates={}

  collision.tileCollision.previous={};
  collision.tileCollision.previous.tileLeft =false;
  collision.tileCollision.previous.tileRight=false;
  collision.tileCollision.previous.tileUp   =false;

  collision.callbackFunctions={};
  collision.callbackFunctions.TileUp    =nil; --Arguments are: newPosition (world)
  collision.callbackFunctions.TileDown  =nil; --Arguments are: newPosition (world) and Angle (unsigned)
  collision.callbackFunctions.TileLeft  =nil; --Arguments are: newPosition (world)
  collision.callbackFunctions.TileRight =nil; --Arguments are: newPosition (world)

		--PrimaryBox
		collision.entityCollision.boxID.CBOX_PRIME=0;

		--Entity collision
		collision.entityCollision.boxID.CBOX_ENT_UP=1;
		collision.entityCollision.boxID.CBOX_ENT_LEFT=2;
		collision.entityCollision.boxID.CBOX_ENT_RIGHT=3;
		collision.entityCollision.boxID.CBOX_ENT_DOWN=4;

		collision.tileCollision.boxID.CBOX_TILE_UP=5;
		collision.tileCollision.boxID.CBOX_TILE_LEFT=6;
		collision.tileCollision.boxID.CBOX_TILE_RIGHT=7;
		collision.tileCollision.boxID.CBOX_TILE_DOWN_A=8;
		collision.tileCollision.boxID.CBOX_TILE_DOWN_B=9;

    --Where CBOXs are relative to ground (Down) position
    --Should check horizontal boxes first
    CBOX_FEET_OFFSET=8;

    collision.tileCollision.coordinates.CBOX_GROUND_A_X_OFFSET =  2
    collision.tileCollision.coordinates.CBOX_GROUND_B_X_OFFSET =  2;
    collision.tileCollision.coordinates.CBOX_GROUND_Y_OFFSET   =  CBOX_FEET_OFFSET;
    collision.tileCollision.coordinates.CBOX_GROUND_H_OFFSET   =  1+CBOX_FEET_OFFSET;
    collision.tileCollision.coordinates.CBOX_GROUND_ORDER      =  5;

    collision.tileCollision.coordinates.CBOX_RIGHT_X_OFFSET    =  0
    collision.tileCollision.coordinates.CBOX_RIGHT_Y_OFFSET    =  4
    collision.tileCollision.coordinates.CBOX_RIGHT_W_OFFSET    =  1
    collision.tileCollision.coordinates.CBOX_RIGHT_H_OFFSET    =  8;
    collision.tileCollision.coordinates.CBOX_RIGHT_ORDER       =  15;

    collision.tileCollision.coordinates.CBOX_LEFT_X_OFFSET     =  0;
    collision.tileCollision.coordinates.CBOX_LEFT_Y_OFFSET     =  4;
    collision.tileCollision.coordinates.CBOX_LEFT_W_OFFSET     =  -1;
    collision.tileCollision.coordinates.CBOX_LEFT_H_OFFSET     =  8;
    collision.tileCollision.coordinates.CBOX_LEFT_ORDER        =  15;

    collision.tileCollision.coordinates.CBOX_UP_Y_OFFSET       =  0;
    collision.tileCollision.coordinates.CBOX_UP_H_OFFSET       =  -1;
    collision.tileCollision.coordinates.CBOX_UP_W_OFFSET       =  2;
    collision.tileCollision.coordinates.CBOX_UP_X_OFFSET       =  1;
    collision.tileCollision.coordinates.CBOX_UP_ORDER          =  10

	--contatins Rect shapes
	collision.boxRect = {}


--Collision Boxes
		--Entity Collision Boxes
			collision.entityCollision.cboxPrimary=nil;

			collision.entityCollision.cboxEntRight=nil;
			collision.entityCollision.cboxEntLeft=nil;
			collision.entityCollision.cboxEntUp=nil;
			collision.entityCollision.cboxEntDown=nil;

		--Tile Collision Boxes
			collision.tileCollision.cboxTileRight=nil;
			collision.tileCollision.cboxTileLeft=nil;
			collision.tileCollision.cboxTileUp=nil;
			collision.tileCollision.cboxTileDownA=nil;
			collision.tileCollision.cboxTileDownB=nil;

		--Collision state variables
			collision.frameProperties={}
      collision.frameProperties.highestHeight=0;
      collision.frameProperties.lowestAngle=0;
			collision.frameProperties.firstCollision=false;


function collision.AngleToSignedAngle(a)
  if(a>180)then
    a= a - 360;
  end
  return a;
end

function collision.Init(w,h, iface, component, eid)
  --mobdebug.start()
  collision.SetWidthHeight(w,h);
  collision.interface=iface;
  collision.EID = eid
  collision.collisionComponent=component

  local coords=collision.tileCollision.coordinates;
  local tboxID = collision.tileCollision.boxID
  --Boxes are to the 'right', 'left', 'up', and 'down' in the absolute sense, not relative to rotation mode or motion
	collision.boxRect[tboxID.CBOX_TILE_RIGHT]= CPP.Rect(coords.CBOX_RIGHT_X_OFFSET,	coords.CBOX_RIGHT_Y_OFFSET,	coords.CBOX_RIGHT_W_OFFSET,	coords.CBOX_RIGHT_H_OFFSET);
	collision.boxRect[tboxID.CBOX_TILE_LEFT]=	 CPP.Rect(coords.CBOX_LEFT_X_OFFSET,   coords.CBOX_LEFT_Y_OFFSET,   coords.CBOX_LEFT_W_OFFSET, 	coords.CBOX_LEFT_H_OFFSET );
	collision.boxRect[tboxID.CBOX_TILE_UP]=		 CPP.Rect(coords.CBOX_UP_X_OFFSET,   	coords.CBOX_UP_Y_OFFSET,	    coords.CBOX_UP_W_OFFSET,     coords.CBOX_UP_H_OFFSET   );

	collision.boxRect[tboxID.CBOX_TILE_DOWN_A]= CPP.Rect(coords.CBOX_GROUND_A_X_OFFSET,	coords.CBOX_GROUND_Y_OFFSET, 	1,	coords.CBOX_GROUND_H_OFFSET);
	collision.boxRect[tboxID.CBOX_TILE_DOWN_B]= CPP.Rect(coords.CBOX_GROUND_B_X_OFFSET,	coords.CBOX_GROUND_Y_OFFSET, 	1,  coords.CBOX_GROUND_H_OFFSET);


  local eboxID = collision.entityCollision.boxID
  collision.boxRect[eboxID.CBOX_PRIME] = CPP.Rect(0, 0, w+4,	h+4);
  collision.boxRect[eboxID.CBOX_ENT_LEFT]=nil;
  collision.boxRect[eboxID.CBOX_ENT_UP]=nil;
  collision.boxRect[eboxID.CBOX_ENT_RIGHT]=nil;
  collision.boxRect[eboxID.CBOX_ENT_DOWN]=CPP.Rect(0, (h/2)+2, w,	h/2);

  local boxRect = collision.boxRect;
	component:AddCollisionBox(boxRect[eboxID.CBOX_PRIME], eboxID.CBOX_PRIME, 0);
	component:CheckForEntities(eboxID.CBOX_PRIME);
	component:SetPrimaryCollisionBox(eboxID.CBOX_PRIME);

    component:AddCollisionBox(boxRect[eboxID.CBOX_ENT_DOWN], eboxID.CBOX_ENT_DOWN, 0);
	component:CheckForEntities(eboxID.CBOX_ENT_DOWN);

	component:AddCollisionBox(boxRect[tboxID.CBOX_TILE_DOWN_A], tboxID.CBOX_TILE_DOWN_A, coords.CBOX_GROUND_ORDER);
	component:CheckForTiles(tboxID.CBOX_TILE_DOWN_A);

	component:AddCollisionBox(boxRect[tboxID.CBOX_TILE_DOWN_B], tboxID.CBOX_TILE_DOWN_B, coords.CBOX_GROUND_ORDER);
	component:CheckForTiles(tboxID.CBOX_TILE_DOWN_B);

	component:AddCollisionBox(boxRect[tboxID.CBOX_TILE_DOWN_B], tboxID.CBOX_TILE_RIGHT, coords.CBOX_RIGHT_ORDER);
	component:CheckForTiles(tboxID.CBOX_TILE_RIGHT);

	component:AddCollisionBox(boxRect[tboxID.CBOX_TILE_DOWN_B], tboxID.CBOX_TILE_LEFT, coords.CBOX_LEFT_ORDER);
	component:CheckForTiles(tboxID.CBOX_TILE_LEFT);

	component:AddCollisionBox(boxRect[tboxID.CBOX_TILE_DOWN_B], tboxID.CBOX_TILE_UP, coords.CBOX_UP_ORDER);
	component:CheckForTiles(tboxID.CBOX_TILE_UP);

	collision.groundTouch=false;
end

function collision.SetWidthHeight(w,h)
  collision.WIDTH=w;
  collision.HEIGHT=h;
  collision.tileCollision.coordinates.CBOX_GROUND_A_X_OFFSET =  w-2
  collision.tileCollision.coordinates.CBOX_GROUND_B_X_OFFSET =  2;
  collision.tileCollision.coordinates.CBOX_GROUND_Y_OFFSET   =  h-CBOX_FEET_OFFSET;
  collision.tileCollision.coordinates.CBOX_GROUND_H_OFFSET   =  1+CBOX_FEET_OFFSET;
  collision.tileCollision.coordinates.CBOX_GROUND_ORDER      =  5;

  --The left and right collision boxes are closer to the top than the bottom
  --this is to allow for covering rougher terrain without the left and right
  --collision boxes incorrectly setting off a collision event
  --A happy side effect is that the character feet will land on the ledge at a
  --higher y coordinate (lower on the screen) than they would otherwise
  --(when such a collision would result in left or right firing instead of feet)

  collision.tileCollision.coordinates.CBOX_RIGHT_X_OFFSET    =  w
  collision.tileCollision.coordinates.CBOX_RIGHT_Y_OFFSET    =  6
  collision.tileCollision.coordinates.CBOX_RIGHT_W_OFFSET    =  1
  collision.tileCollision.coordinates.CBOX_RIGHT_H_OFFSET    =  h-14;
  collision.tileCollision.coordinates.CBOX_RIGHT_ORDER       =  15;

  collision.tileCollision.coordinates.CBOX_LEFT_X_OFFSET     =  0;
  collision.tileCollision.coordinates.CBOX_LEFT_Y_OFFSET     =  6;
  collision.tileCollision.coordinates.CBOX_LEFT_W_OFFSET     =  -1;
  collision.tileCollision.coordinates.CBOX_LEFT_H_OFFSET     =  h-14;
  collision.tileCollision.coordinates.CBOX_LEFT_ORDER        =  15;

  collision.tileCollision.coordinates.CBOX_UP_Y_OFFSET       =  0;
  collision.tileCollision.coordinates.CBOX_UP_H_OFFSET       =  -1;
  collision.tileCollision.coordinates.CBOX_UP_W_OFFSET       =  (w/2);
  collision.tileCollision.coordinates.CBOX_UP_X_OFFSET       =  (w/4);
  collision.tileCollision.coordinates.CBOX_UP_ORDER          =  10;
end

--returns true false
function collision.GetHeightMapValue(absoluteX, tileCollisionPacket)
    local cbox_value = 0;
    local boxid=tileCollisionPacket:GetID();
    local hmap=tileCollisionPacket:GetHmap();
    local HMAP_index_value= 0;
    local tx=tileCollisionPacket:GetX();

    --First, figure out the x-coordinate of the heightmap value (height map index value)
    if(boxid==collision.tileCollision.boxID.CBOX_TILE_DOWN_A) then
     cbox_value = collision.tileCollision.coordinates.CBOX_GROUND_A_X_OFFSET;
    elseif(boxid==collision.tileCollision.boxID.CBOX_TILE_DOWN_B) then
     cbox_value = collision.tileCollision.coordinates.CBOX_GROUND_B_X_OFFSET;
    end

    --Get the world x position of the collision box
    cbox_value= cbox_value + absoluteX;
    HMAP_index_value= cbox_value - (tx*collision.TILEWIDTH);

    --Got the heightmap index value, now actually get the height value and set the proper y-value
    if((HMAP_index_value>15)or(HMAP_index_value<0))then
      collision.interface:WriteError(collision.EID, "Uh-Oh, HMAP Value out of bounds");
      collision.interface:WriteError(collision.EID, tostring(HMAP_index_value));
      return;
    end
    return hmap:GetHeightMapH( HMAP_index_value );
end

function collision.Update(xspd, yspd)
	if(collision.groundTouch==true)then
   		collision.tileCollision.footHeightValue=math.floor(yspd+0.5+CBOX_FEET_OFFSET+math.abs(xspd))+2
 	else
   		collision.tileCollision.footHeightValue=math.floor(yspd + 0.5 + CBOX_FEET_OFFSET)+1
  	end

	local tboxID = collision.tileCollision.boxID
	local eboxID = collision.entityCollision.boxID
	local boxes = collision.boxRect
	boxes[tboxID.CBOX_TILE_DOWN_A].h 	= collision.tileCollision.footHeightValue
	boxes[tboxID.CBOX_TILE_DOWN_B].h 	= collision.tileCollision.footHeightValue
	boxes[tboxID.CBOX_TILE_UP].h 		= math.floor(yspd - 2.5)
	boxes[tboxID.CBOX_TILE_LEFT].w 		= math.floor(xspd - 0.5)-1
	boxes[tboxID.CBOX_TILE_RIGHT].w 		= math.floor(xspd + 0.5)

	collision.collisionComponent:SetShape(tboxID.CBOX_TILE_DOWN_A, 	boxes[tboxID.CBOX_TILE_DOWN_A]);
	collision.collisionComponent:SetShape(tboxID.CBOX_TILE_DOWN_B, 	boxes[tboxID.CBOX_TILE_DOWN_B]);
	collision.collisionComponent:SetShape(tboxID.CBOX_TILE_LEFT, 	boxes[tboxID.CBOX_TILE_LEFT]);
	collision.collisionComponent:SetShape(tboxID.CBOX_TILE_RIGHT,  	boxes[tboxID.CBOX_TILE_RIGHT]);
	collision.collisionComponent:SetShape(tboxID.CBOX_TILE_UP,  	boxes[tboxID.CBOX_TILE_UP]);

	--[[
  	collision.collisionComponent:ChangeHeight(collision.tileCollision.boxID.CBOX_TILE_DOWN_A,  collision.tileCollision.footHeightValue);
  	collision.collisionComponent:ChangeHeight(collision.tileCollision.boxID.CBOX_TILE_DOWN_B,  collision.tileCollision.footHeightValue);
  	collision.collisionComponent:ChangeHeight(collision.tileCollision.boxID.CBOX_TILE_UP,      math.floor(yspd - 2.5));
	collision.collisionComponent:ChangeWidth(collision.tileCollision.boxID.CBOX_TILE_LEFT,    math.floor(xspd - 0.5)-1);
	collision.collisionComponent:ChangeWidth(collision.tileCollision.boxID.CBOX_TILE_RIGHT,   math.floor(xspd + 0.5));
	]]--

  	collision.tileCollision.previous.tileLeft =false;
  	collision.tileCollision.previous.tileRight=false;
  	collision.tileCollision.previous.tileUp   =false;
  	collision.ignoreTileX=-1;
  	collision.prevGroundTouch=collision.groundTouch;
	collision.groundTouch=false;
	collision.frameProperties.firstCollision=false;
end

function collision.OnTileCollision(packet, hspd, vspd, absoluteX, absoluteY)
  --Easy access to context data
  local boxid=packet:GetID();
	local tx=packet:GetX();
	local ty=packet:GetY();
	local layer=packet:GetLayer();
	local hmap=packet:GetHmap();
  local usesHMaps=layer:UsesHMaps();
  local newPosition;
  local xval=absoluteX;
  local yval=absoluteY;

  --Commonly used variables
	local HMAPheight=0;
	local frameheight=ty * collision.TILEHEIGHT;

  if(collision.frameProperties.firstCollision==false)then
    collision.frameProperties.highestHeight=frameheight + 1000;
    collision.frameProperties.lowestAngle=360;
		collision.frameProperties.firstCollision=true;
	end

  --============================--
	--If Ground Collision Occurred--
	--============================--
  --if you're already on the ground or moving downward
	if ( ((boxid==collision.tileCollision.boxID.CBOX_TILE_DOWN_A) or (boxid==collision.tileCollision.boxID.CBOX_TILE_DOWN_B))
     --and ((collision.groundTouch==true)or(vspd>=0))
     and(tx~=collision.ignoreTileX)
     ) then

    local thisAngle=hmap.angleH;
    local thisAngleSigned= collision.AngleToSignedAngle(thisAngle);
    local maximumFootY=collision.tileCollision.footHeightValue + absoluteY + collision.tileCollision.coordinates.CBOX_GROUND_Y_OFFSET;
    local HMAPheight= collision.GetHeightMapValue(absoluteX, packet);

    --Don't register a collision if there isn't any height value
    if (HMAPheight==0 or HMAPheight==nil) then return; end
    --This line of code stops you from clipping through objects you're riding on to land on the tile beneath you
    if ((ty*16)-HMAPheight+16)>maximumFootY then return; end

    frameheight = frameheight - HMAPheight;
    if(frameheight>collision.frameProperties.highestHeight)then
      return; --Only stand on the highest height value found

    elseif(collision.frameProperties.highestHeight==frameheight)then
      if( (math.abs(collision.frameProperties.lowestAngle)<=math.abs(thisAngleSigned)) )then
        return; --if the heights are the same, only stand on the lowest angle
      end
    end

    --Update position
    newPosition=CPP.Coord2df(xval, ( (ty+1) *16 ) - collision.HEIGHT - HMAPheight);

    --Update variables
    collision.frameProperties.lowestAngle=math.abs(thisAngleSigned);
		collision.frameProperties.highestHeight=frameheight;
    collision.groundTouch=true;

    collision.callbackFunctions.TileDown(newPosition, thisAngle);

   --===========================--
	--If Right Collision Occurred--
	--===========================--

  elseif ( (boxid==collision.tileCollision.boxID.CBOX_TILE_RIGHT) ) then
    if(usesHMaps)then
      return;
    end
    if((hspd>=0) or ((collision.groundTouch==false) and (hspd==0)))  then
      --Add one because the first x pixel counts as part of the width
      newPosition=CPP.Coord2df( (tx*collision.TILEWIDTH)-collision.tileCollision.coordinates.CBOX_RIGHT_X_OFFSET-collision.tileCollision.coordinates.CBOX_RIGHT_W_OFFSET+1,                                                                                                     yval);

      collision.tileCollision.previous.tileRight=true;
      --The top collision box won't try to collide with this tile for the rest of the frame
      --It's impossible, because since you just collided with the x-coord to your right and were pushed back, you
      --can't possibly also have it above you
      collision.ignoreTileX=tx;

      collision.callbackFunctions.TileRight(newPosition);
    end

  --==========================--
	--If Left Collision Occurred--
	--==========================--

  elseif ( (boxid==collision.tileCollision.boxID.CBOX_TILE_LEFT) ) then
    if(usesHMaps)then
      return;
    end
    if((hspd<=0) or ((collision.groundTouch==false) and (hspd==0))) then
      --Subtract one because (tx+1) pushes one pixel past the actual tile colided with
      newPosition=CPP.Coord2df(((tx+1)*collision.TILEWIDTH)-1, yval);

      collision.tileCollision.previous.tileLeft=true;
      --The top collision box won't try to collide with this tile for the rest of the frame
      --It's impossible, because since you just collided with the x-coord to your left and were pushed back, you
      --can't possibly also have it above you
      collision.ignoreTileX=tx;
      collision.callbackFunctions.TileLeft(newPosition);
    end

  --=========================--
	--If Top Collision Occurred--
	--=========================--

  elseif ( (boxid==collision.tileCollision.boxID.CBOX_TILE_UP) and(tx~=collision.ignoreTileX))then
    --Subtract one because (ty+1) pushes one pixel past the actual tile colided with
    if(usesHMaps)then
      return;
    end
    newPosition=CPP.Coord2df(xval, ((ty+1)*collision.TILEHEIGHT)-1);

    collision.tileCollision.previous.tileUp   =true;

    collision.callbackFunctions.TileUp(newPosition);
  end

end


--return container
return collision;
