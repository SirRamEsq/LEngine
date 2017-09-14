local collision = {}
collision={};

collision.cpp=nil;
collision.cComp=nil;

	--this tile coordinate will be ignored by top and bottom colliders
	--if the coordinate is hit by either the left or right collision boxes
	collision.ignoreTileX=-1;
	collision.WIDTH=0;
	collision.HEIGHT=0;
	collision.TILEWIDTH=16;
	collision.TILEHEIGHT=16;

	collision.footHeightValue=0
	collision.boxID={}
	collision.coordinates={}

	collision.previous={};
	collision.previous.tileLeft =false;
	collision.previous.tileRight=false;
	collision.previous.tileUp	=false;

	collision.callbackFunctions={};
	collision.callbackFunctions.TileUp		=nil; --Arguments are: newPosition (world)
	collision.callbackFunctions.TileDown	=nil; --Arguments are: newPosition (world) and Angle (unsigned)
	collision.callbackFunctions.TileLeft	=nil; --Arguments are: newPosition (world)
	collision.callbackFunctions.TileRight 	=nil; --Arguments are: newPosition (world)

	collision.boxID.TILE_UP=5;
	collision.boxID.TILE_LEFT=6;
	collision.boxID.TILE_RIGHT=7;
	collision.boxID.TILE_LEFT_SHORT=16;
	collision.boxID.TILE_RIGHT_SHORT=17;
	collision.boxID.TILE_DOWN_R=8;
	collision.boxID.TILE_DOWN_L=9;

		--Where BOXs are relative to ground (Down) position
		--Should check horizontal boxes first
		collision.FEET_OFFSET=8;

		collision.coordinates.GROUND_R_X_OFFSET =  2
		collision.coordinates.GROUND_L_X_OFFSET =  -2
		collision.coordinates.GROUND_Y_OFFSET	=  collision.FEET_OFFSET;
		collision.coordinates.GROUND_H_OFFSET	=  1+collision.FEET_OFFSET;
		collision.coordinates.GROUND_ORDER		=  5;

		collision.coordinates.RIGHT_X_OFFSET	=  0
		collision.coordinates.RIGHT_Y_OFFSET	=  4
		collision.coordinates.RIGHT_W_OFFSET	=  1
		collision.coordinates.RIGHT_H_OFFSET	=  8;
		collision.coordinates.RIGHT_ORDER		=  15;

		collision.coordinates.LEFT_X_OFFSET		=  0;
		collision.coordinates.LEFT_Y_OFFSET		=  4;
		collision.coordinates.LEFT_W_OFFSET		=  -1;
		collision.coordinates.LEFT_H_OFFSET		=  8;
		collision.coordinates.LEFT_ORDER		=  15;

		collision.coordinates.RIGHT_SHORT_Y_OFFSET	=  11
		collision.coordinates.RIGHT_SHORT_H_OFFSET	=  4;

		collision.coordinates.LEFT_SHORT_Y_OFFSET	=  11;
		collision.coordinates.LEFT_SHORT_H_OFFSET	=  4;

		collision.coordinates.UP_Y_OFFSET		=  0;
		collision.coordinates.UP_H_OFFSET		=  -1;
		collision.coordinates.UP_W_OFFSET		=  2;
		collision.coordinates.UP_X_OFFSET		=  1;
		collision.coordinates.UP_ORDER			=  10


--Collision Boxes
		--Tile Collision Boxes
			collision.boxTileRight=nil;
			collision.boxTileLeft=nil;
			collision.boxTileUp=nil;
			collision.boxTileDownA=nil;
			collision.boxTileDownB=nil;

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

--can replace iface with an errorlogging call back
function collision.Init(w, h, iface, component, eid)
	collision.SetWidthHeight(w,h);
	collision.cpp=iface;
	collision.EID = eid
	collision.cComp=component

	local coords=collision.coordinates;
	--Boxes are to the 'right', 'left', 'up', and 'down' in the absolute sense, not relative to rotation mode or motion
	collision.boxTileRight= CPP.CRect(coords.RIGHT_X_OFFSET,	coords.RIGHT_Y_OFFSET,	coords.RIGHT_W_OFFSET,	coords.RIGHT_H_OFFSET);
	collision.boxTileLeft=	 CPP.CRect(coords.LEFT_X_OFFSET,	 coords.LEFT_Y_OFFSET,	 coords.LEFT_W_OFFSET,	coords.LEFT_H_OFFSET );
	collision.boxTileRightShort= CPP.CRect(coords.RIGHT_X_OFFSET,	coords.RIGHT_SHORT_Y_OFFSET, coords.RIGHT_W_OFFSET,	coords.RIGHT_SHORT_H_OFFSET);
	collision.boxTileLeftShort=	 CPP.CRect(coords.LEFT_X_OFFSET,	coords.LEFT_SHORT_Y_OFFSET,	 coords.LEFT_W_OFFSET,	coords.LEFT_SHORT_H_OFFSET );
	collision.boxTileUp=		 CPP.CRect(coords.UP_X_OFFSET,		coords.UP_Y_OFFSET,			coords.UP_W_OFFSET,			coords.UP_H_OFFSET	 );

	collision.boxTileDownR= CPP.CRect(coords.GROUND_R_X_OFFSET,	coords.GROUND_Y_OFFSET,		1,	coords.GROUND_H_OFFSET);
	collision.boxTileDownL= CPP.CRect(coords.GROUND_L_X_OFFSET,	coords.GROUND_Y_OFFSET,		1,	coords.GROUND_H_OFFSET);

	collision.cComp:AddCollisionBox(collision.boxTileDownR, collision.boxID.TILE_DOWN_R, coords.GROUND_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_DOWN_R);

	collision.cComp:AddCollisionBox(collision.boxTileDownL, collision.boxID.TILE_DOWN_L, coords.GROUND_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_DOWN_L);

	collision.cComp:AddCollisionBox(collision.boxTileRight, collision.boxID.TILE_RIGHT, coords.RIGHT_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_RIGHT);

	collision.cComp:AddCollisionBox(collision.boxTileLeft, collision.boxID.TILE_LEFT, coords.LEFT_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_LEFT);

	collision.cComp:AddCollisionBox(collision.boxTileRightShort, collision.boxID.TILE_RIGHT_SHORT, coords.RIGHT_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_RIGHT_SHORT);
	collision.cComp:Deactivate(collision.boxID.TILE_RIGHT_SHORT);

	collision.cComp:AddCollisionBox(collision.boxTileLeftShort, collision.boxID.TILE_LEFT_SHORT, coords.LEFT_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_LEFT_SHORT);
	collision.cComp:Deactivate(collision.boxID.TILE_RIGHT_SHORT);

	collision.cComp:AddCollisionBox(collision.boxTileUp, collision.boxID.TILE_UP, coords.UP_ORDER);
	collision.cComp:CheckForTiles(collision.boxID.TILE_UP);

	collision.groundTouch=false;
end

function collision.SetWidthHeight(w, h)
	--louie is 18,32
	collision.WIDTH=w;
	collision.HEIGHT=h;
	collision.coordinates.GROUND_R_X_OFFSET 	=  w-2
	collision.coordinates.GROUND_L_X_OFFSET 	=  2;
	collision.coordinates.GROUND_Y_OFFSET		=  h-collision.FEET_OFFSET;
	collision.coordinates.GROUND_H_OFFSET		=  1+collision.FEET_OFFSET;
	collision.coordinates.GROUND_ORDER			=  5;

	--The left and right collision boxes are closer to the top than the bottom
	--this is to allow for covering rougher terrain without the left and right
	--collision boxes incorrectly setting off a collision event
	--A happy side effect is that the character feet will land on the ledge at a
	--higher y coordinate (lower on the screen) than they would otherwise
	--(when such a collision would result in left or right firing instead of feet)

	collision.coordinates.RIGHT_X_OFFSET		=  w
	collision.coordinates.RIGHT_Y_OFFSET		=  9
	collision.coordinates.RIGHT_W_OFFSET		=  1
	collision.coordinates.RIGHT_H_OFFSET		=  h-14;
	collision.coordinates.RIGHT_ORDER			=  15;

	collision.coordinates.LEFT_X_OFFSET			=  0;
	collision.coordinates.LEFT_Y_OFFSET			=  9
	collision.coordinates.LEFT_W_OFFSET			=  -1;
	collision.coordinates.LEFT_H_OFFSET			=  h-14;
	collision.coordinates.LEFT_ORDER			=  15;

	collision.coordinates.RIGHT_SHORT_Y_OFFSET	=  math.floor(h/2) - 1
	collision.coordinates.RIGHT_SHORT_H_OFFSET	=  math.floor(h/4)

	collision.coordinates.LEFT_SHORT_Y_OFFSET	=  9
	collision.coordinates.LEFT_SHORT_H_OFFSET	=  h-14;

	collision.coordinates.UP_Y_OFFSET			=  8;
	collision.coordinates.UP_H_OFFSET			=  1;
	collision.coordinates.UP_W_OFFSET			=  math.floor(w/2);
	collision.coordinates.UP_X_OFFSET			=  math.floor(w/4);
	collision.coordinates.UP_ORDER				=  10;
end

function collision.GetHeightMapValue(absoluteX, tileCollisionPacket)
	if(tileCollisionPacket:GetLayer():UsesHMaps() == false) then return 16 end
	local box_value = 0;
	local boxid=tileCollisionPacket:GetID();
	local hmap=tileCollisionPacket:GetHmap();
	local HMAP_index_value= 0;
	local tx=tileCollisionPacket:GetX();

	--First, figure out the x-coordinate of the heightmap value (height map index value)
	if(boxid==collision.boxID.TILE_DOWN_R) then
		box_value = collision.coordinates.GROUND_R_X_OFFSET;
	elseif(boxid==collision.boxID.TILE_DOWN_L) then
		box_value = collision.coordinates.GROUND_L_X_OFFSET;
	end

	--Get the world x position of the collision box
	box_value= box_value + absoluteX;
	HMAP_index_value= box_value - (tx*collision.TILEWIDTH);

	--Got the heightmap index value, now actually get the height value and set the proper y-value
	if((HMAP_index_value>15)or(HMAP_index_value<0))then
		collision.cpp:WriteError(collision.EID, "Uh-Oh, index '" .. HMAP_index_value .. "' is out of bounds with boxValue '" .. box_value .. "' and tx '" .. tx .. "'");
		collision.cpp:WriteError(collision.EID, tostring(HMAP_index_value));
		return;
	end

	return hmap:GetHeightMapH( HMAP_index_value );
end

function collision.Update(xspd, yspd)
	if(collision.groundTouch==true)then
		collision.footHeightValue=math.floor(yspd+0.5+collision.FEET_OFFSET+math.abs(xspd))+2
	else
		collision.footHeightValue=math.floor(yspd + 0.5 + collision.FEET_OFFSET)+1
	end

	collision.cComp:ChangeHeight(collision.boxID.TILE_DOWN_R,  collision.footHeightValue);
	collision.cComp:ChangeHeight(collision.boxID.TILE_DOWN_L,  collision.footHeightValue);
	collision.cComp:ChangeHeight(collision.boxID.TILE_UP,		math.floor(yspd - 2.5));
	collision.cComp:ChangeWidth(collision.boxID.TILE_LEFT,		math.floor(xspd - 0.5));
	collision.cComp:ChangeWidth(collision.boxID.TILE_RIGHT,		math.floor(xspd + 0.5));

	collision.previous.tileLeft =false;
	collision.previous.tileRight=false;
	collision.previous.tileUp		=false;
	collision.ignoreTileX=-1;
	collision.prevGroundTouch=collision.groundTouch;
	collision.groundTouch=false;
	collision.frameProperties.firstCollision=false;
end

function collision.OnTileCollision(packet, hspd, vspd, exactX, exactY)
	--[[
	The fact that this function has been called means that the CPP engine has decided a collision exists here
	]]--

	--Easy access to context data
	local boxid=packet:GetID();
	local tx=packet:GetX();
	local ty=packet:GetY();
	local layer=packet:GetLayer();
	local hmap=packet:GetHmap();
	local usesHMaps=layer:UsesHMaps();
	local newPosition;
	local xval=exactX;
	local yval=exactY;

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
	if ( ((boxid==collision.boxID.TILE_DOWN_R) or (boxid==collision.boxID.TILE_DOWN_L))
		 and(tx~=collision.ignoreTileX)
		 ) then

		local thisAngle=hmap.angleH;
		local thisAngleSigned= collision.AngleToSignedAngle(thisAngle);
		local maximumFootY=collision.footHeightValue + exactY + collision.coordinates.GROUND_Y_OFFSET;
		local HMAPheight= collision.GetHeightMapValue(exactX, packet);

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

		collision.callbackFunctions.TileDown(newPosition, thisAngle, layer, tx, ty);

	--===========================--
	--If Right Collision Occurred--
	--===========================--
	elseif ( (boxid==collision.boxID.TILE_RIGHT) or (boxid == collision.boxID.TILE_RIGHT_SHORT)) then
		if(usesHMaps)then
			return;
		end
		if((hspd>=0) or ((collision.groundTouch==false) and (hspd==0)))  then
			--Add one because the first x pixel counts as part of the width
			newPosition=CPP.Coord2df( (tx*collision.TILEWIDTH)-collision.coordinates.RIGHT_X_OFFSET-collision.coordinates.RIGHT_W_OFFSET+1,																																																			yval);

			collision.previous.tileRight=true;
			--The top collision box won't try to collide with this tile for the rest of the frame
			--It's impossible, because since you just collided with the x-coord to your right and were pushed back, you
			--can't possibly also have it above you
			collision.ignoreTileX=tx;

			collision.callbackFunctions.TileRight(newPosition, layer, tx, ty);
		end

	--==========================--
	--If Left Collision Occurred--
	--==========================--
	elseif ( (boxid==collision.boxID.TILE_LEFT) or (boxid == collision.boxID.TILE_LEFT_SHORT) ) then
		if(usesHMaps)then
			return;
		end
		if((hspd<=0) or ((collision.groundTouch==false) and (hspd==0))) then
			--Subtract one because (tx+1) pushes one pixel past the actual tile colided with
			newPosition=CPP.Coord2df(((tx+1)*collision.TILEWIDTH)-1, yval);

			collision.previous.tileLeft=true;
			--The top collision box won't try to collide with this tile for the rest of the frame
			--It's impossible, because since you just collided with the x-coord to your left and were pushed back, you
			--can't possibly also have it above you
			collision.ignoreTileX=tx;
			collision.callbackFunctions.TileLeft(newPosition, layer, tx, ty);
		end

	--=========================--
	--If Top Collision Occurred--
	--=========================--
	elseif ( (boxid==collision.boxID.TILE_UP) and(tx~=collision.ignoreTileX))then
		--Subtract one because (ty+1) pushes one pixel past the actual tile colided with
		if(usesHMaps)then
			return;
		end
		newPosition=CPP.Coord2df(xval, ((ty+1)*collision.TILEHEIGHT)-1);

		collision.previous.tileUp		=true;

		collision.callbackFunctions.TileUp(newPosition, layer, tx, ty);
	end
end


--return container
return collision;
