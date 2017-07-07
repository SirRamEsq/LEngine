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
function collision.Init(self, w, h, iface, component, eid)
	self:SetWidthHeight(w,h);
	self.cpp=iface;
	self.EID = eid
	self.cComp=component

	local coords=self.coordinates;
	--Boxes are to the 'right', 'left', 'up', and 'down' in the absolute sense, not relative to rotation mode or motion
	self.boxTileRight= CPP.CRect(coords.RIGHT_X_OFFSET,	coords.RIGHT_Y_OFFSET,	coords.RIGHT_W_OFFSET,	coords.RIGHT_H_OFFSET);
	self.boxTileLeft=	 CPP.CRect(coords.LEFT_X_OFFSET,	 coords.LEFT_Y_OFFSET,	 coords.LEFT_W_OFFSET,	coords.LEFT_H_OFFSET );
	self.boxTileUp=		 CPP.CRect(coords.UP_X_OFFSET,		coords.UP_Y_OFFSET,			coords.UP_W_OFFSET,			coords.UP_H_OFFSET	 );

	self.boxTileDownR= CPP.CRect(coords.GROUND_R_X_OFFSET,	coords.GROUND_Y_OFFSET,		1,	coords.GROUND_H_OFFSET);
	self.boxTileDownL= CPP.CRect(coords.GROUND_L_X_OFFSET,	coords.GROUND_Y_OFFSET,		1,	coords.GROUND_H_OFFSET);

	self.cComp:AddCollisionBox(self.boxTileDownR, self.boxID.TILE_DOWN_R, coords.GROUND_ORDER);
	self.cComp:CheckForTiles(self.boxID.TILE_DOWN_R);

	self.cComp:AddCollisionBox(self.boxTileDownL, self.boxID.TILE_DOWN_L, coords.GROUND_ORDER);
	self.cComp:CheckForTiles(self.boxID.TILE_DOWN_L);

	self.cComp:AddCollisionBox(self.boxTileRight, self.boxID.TILE_RIGHT, coords.RIGHT_ORDER);
	self.cComp:CheckForTiles(self.boxID.TILE_RIGHT);

	self.cComp:AddCollisionBox(self.boxTileLeft, self.boxID.TILE_LEFT, coords.LEFT_ORDER);
	self.cComp:CheckForTiles(self.boxID.TILE_LEFT);

	self.cComp:AddCollisionBox(self.boxTileUp, self.boxID.TILE_UP, coords.UP_ORDER);
	self.cComp:CheckForTiles(self.boxID.TILE_UP);

	self.groundTouch=false;
end

function collision.SetWidthHeight(self, w, h)
	self.WIDTH=w;
	self.HEIGHT=h;
	self.coordinates.GROUND_R_X_OFFSET =  w-2
	self.coordinates.GROUND_L_X_OFFSET =  2;
	self.coordinates.GROUND_Y_OFFSET		=  h-self.FEET_OFFSET;
	self.coordinates.GROUND_H_OFFSET		=  1+self.FEET_OFFSET;
	self.coordinates.GROUND_ORDER			=  5;

	--The left and right self boxes are closer to the top than the bottom
	--this is to allow for covering rougher terrain without the left and right
	--self boxes incorrectly setting off a self event
	--A happy side effect is that the character feet will land on the ledge at a
	--higher y coordinate (lower on the screen) than they would otherwise
	--(when such a self would result in left or right firing instead of feet)

	self.coordinates.RIGHT_X_OFFSET		=  w
	self.coordinates.RIGHT_Y_OFFSET		=  6
	self.coordinates.RIGHT_W_OFFSET		=  1
	self.coordinates.RIGHT_H_OFFSET		=  h-14;
	self.coordinates.RIGHT_ORDER				=  15;

	self.coordinates.LEFT_X_OFFSET			=  0;
	self.coordinates.LEFT_Y_OFFSET			=  6
	self.coordinates.LEFT_W_OFFSET			=  -1;
	self.coordinates.LEFT_H_OFFSET			=  h-14;
	self.coordinates.LEFT_ORDER				=  15;

	self.coordinates.UP_Y_OFFSET				=  0;
	self.coordinates.UP_H_OFFSET				=  -1;
	self.coordinates.UP_W_OFFSET				=  (w/2);
	self.coordinates.UP_X_OFFSET				=  (w/4);
	self.coordinates.UP_ORDER					=  10;
end

function collision.GetHeightMapValue(self, absoluteX, tileCollisionPacket)
	if(tileCollisionPacket.layer:UsesHMaps() == true) then return 16 end
	local box_value = 0;
	local boxid=tileCollisionPacket:GetID();
	local hmap=tileCollisionPacket:GetHmap();
	local HMAP_index_value= 0;
	local tx=tileCollisionPacket:GetX();

	--First, figure out the x-coordinate of the heightmap value (height map index value)
	if(boxid==collision.boxID.TILE_DOWN_R) then
		box_value = self.coordinates.GROUND_R_X_OFFSET;
	elseif(boxid==collision.boxID.TILE_DOWN_L) then
		box_value = self.coordinates.GROUND_L_X_OFFSET;
	end

	--Get the world x position of the collision box
	box_value= box_value + absoluteX;
	HMAP_index_value= box_value - (tx*collision.TILEWIDTH);

	--Got the heightmap index value, now actually get the height value and set the proper y-value
	if((HMAP_index_value>15)or(HMAP_index_value<0))then
		self.cpp:WriteError(collision.EID, "Uh-Oh, index '" .. HMAP_index_value .. "' is out of bounds with boxValue '" .. box_value .. "' and tx '" .. tx .. "'");
		self.cpp:WriteError(collision.EID, tostring(HMAP_index_value));
		return;
	end

	return hmap:GetHeightMapH( HMAP_index_value );
end

function collision.Update(self, xspd, yspd)
	if(self.groundTouch==true)then
		self.footHeightValue=math.floor(yspd+0.5+self.FEET_OFFSET+math.abs(xspd))+2
	else
		self.footHeightValue=math.floor(yspd + 0.5 + self.FEET_OFFSET)+1
	end

	self.cComp:ChangeHeight(self.boxID.TILE_DOWN_R,  self.footHeightValue);
	self.cComp:ChangeHeight(self.boxID.TILE_DOWN_L,  self.footHeightValue);
	self.cComp:ChangeHeight(self.boxID.TILE_UP,		math.floor(yspd - 2.5));
	self.cComp:ChangeWidth(self.boxID.TILE_LEFT,		math.floor(xspd - 0.5));
	self.cComp:ChangeWidth(self.boxID.TILE_RIGHT,		math.floor(xspd + 0.5));

	self.previous.tileLeft =false;
	self.previous.tileRight=false;
	self.previous.tileUp		=false;
	self.ignoreTileX=-1;
	self.prevGroundTouch=self.groundTouch;
	self.groundTouch=false;
	self.frameProperties.firstCollision=false;
end

function collision.OnTileCollision(self, packet, hspd, vspd, exactX, exactY)
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
	local frameheight=ty * self.TILEHEIGHT;

	if(self.frameProperties.firstCollision==false)then
		self.frameProperties.highestHeight=frameheight + 1000;
		self.frameProperties.lowestAngle=360;
		self.frameProperties.firstCollision=true;
	end

	--============================--
	--If Ground Collision Occurred--
	--============================--
	if ( ((boxid==self.boxID.TILE_DOWN_R) or (boxid==self.boxID.TILE_DOWN_L))
		 and(tx~=self.ignoreTileX)
		 ) then

		local thisAngle=hmap.angleH;
		local thisAngleSigned= self.AngleToSignedAngle(thisAngle);
		local maximumFootY=self.footHeightValue + exactY + self.coordinates.GROUND_Y_OFFSET;
		local HMAPheight= self:GetHeightMapValue(exactX, packet);

		--Don't register a self if there isn't any height value
		if (HMAPheight==0 or HMAPheight==nil) then return; end
		--This line of code stops you from clipping through objects you're riding on to land on the tile beneath you
		if ((ty*16)-HMAPheight+16)>maximumFootY then return; end

		frameheight = frameheight - HMAPheight;
		if(frameheight>self.frameProperties.highestHeight)then
			return; --Only stand on the highest height value found

		elseif(self.frameProperties.highestHeight==frameheight)then
			if( (math.abs(self.frameProperties.lowestAngle)<=math.abs(thisAngleSigned)) )then
				return; --if the heights are the same, only stand on the lowest angle
			end
		end

		--Update position
		newPosition=CPP.Coord2df(xval, ( (ty+1) *16 ) - self.HEIGHT - HMAPheight);

		--Update variables
		self.frameProperties.lowestAngle=math.abs(thisAngleSigned);
		self.frameProperties.highestHeight=frameheight;
		self.groundTouch=true;

		self.callbackFunctions.TileDown(newPosition, thisAngle);

	--===========================--
	--If Right Collision Occurred--
	--===========================--
	elseif ( (boxid==self.boxID.TILE_RIGHT) ) then
		if(usesHMaps)then
			return;
		end
		if((hspd>=0) or ((self.groundTouch==false) and (hspd==0)))  then
			--Add one because the first x pixel counts as part of the width
			newPosition=CPP.Coord2df( (tx*self.TILEWIDTH)-self.coordinates.RIGHT_X_OFFSET-self.coordinates.RIGHT_W_OFFSET+1,																																																			yval);

			self.previous.tileRight=true;
			--The top self box won't try to collide with this tile for the rest of the frame
			--It's impossible, because since you just collided with the x-coord to your right and were pushed back, you
			--can't possibly also have it above you
			self.ignoreTileX=tx;

			self.callbackFunctions.TileRight(newPosition);
		end

	--==========================--
	--If Left Collision Occurred--
	--==========================--
	elseif ( (boxid==self.boxID.TILE_LEFT) ) then
		if(usesHMaps)then
			return;
		end
		if((hspd<=0) or ((self.groundTouch==false) and (hspd==0))) then
			--Subtract one because (tx+1) pushes one pixel past the actual tile colided with
			newPosition=CPP.Coord2df(((tx+1)*self.TILEWIDTH)-1, yval);

			self.previous.tileLeft=true;
			--The top self box won't try to collide with this tile for the rest of the frame
			--It's impossible, because since you just collided with the x-coord to your left and were pushed back, you
			--can't possibly also have it above you
			self.ignoreTileX=tx;
			self.callbackFunctions.TileLeft(newPosition);
		end

	--=========================--
	--If Top Collision Occurred--
	--=========================--
	elseif ( (boxid==self.boxID.TILE_UP) and(tx~=self.ignoreTileX))then
		--Subtract one because (ty+1) pushes one pixel past the actual tile colided with
		if(usesHMaps)then
			return;
		end
		newPosition=CPP.Coord2df(xval, ((ty+1)*self.TILEHEIGHT)-1);

		self.previous.tileUp		=true;

		self.callbackFunctions.TileUp(newPosition);
	end
end


--return container
return collision;
