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

	collision.boxID.BOX_TILE_UP=5;
	collision.boxID.BOX_TILE_LEFT=6;
	collision.boxID.BOX_TILE_RIGHT=7;
	collision.boxID.BOX_TILE_DOWN_A=8;
	collision.boxID.BOX_TILE_DOWN_B=9;

		--Where BOXs are relative to ground (Down) position
		--Should check horizontal boxes first
		collision.BOX_FEET_OFFSET=8;

		collision.coordinates.BOX_GROUND_A_X_OFFSET =  -2 --right box
		collision.coordinates.BOX_GROUND_B_X_OFFSET =  2 --left box
		collision.coordinates.BOX_GROUND_Y_OFFSET	=  collision.BOX_FEET_OFFSET;
		collision.coordinates.BOX_GROUND_H_OFFSET	=  1+collision.BOX_FEET_OFFSET;
		collision.coordinates.BOX_GROUND_ORDER		=  5;

		collision.coordinates.BOX_RIGHT_X_OFFSET	=  0
		collision.coordinates.BOX_RIGHT_Y_OFFSET	=  4
		collision.coordinates.BOX_RIGHT_W_OFFSET	=  1
		collision.coordinates.BOX_RIGHT_H_OFFSET	=  8;
		collision.coordinates.BOX_RIGHT_ORDER		=  15;

		collision.coordinates.BOX_LEFT_X_OFFSET		=  0;
		collision.coordinates.BOX_LEFT_Y_OFFSET		=  4;
		collision.coordinates.BOX_LEFT_W_OFFSET		=  -1;
		collision.coordinates.BOX_LEFT_H_OFFSET		=  8;
		collision.coordinates.BOX_LEFT_ORDER		=  15;

		collision.coordinates.BOX_UP_Y_OFFSET		=  0;
		collision.coordinates.BOX_UP_H_OFFSET		=  -1;
		collision.coordinates.BOX_UP_W_OFFSET		=  2;
		collision.coordinates.BOX_UP_X_OFFSET		=  1;
		collision.coordinates.BOX_UP_ORDER			=  10


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
	self.boxTileRight= CPP.CRect(coords.BOX_RIGHT_X_OFFSET,	coords.BOX_RIGHT_Y_OFFSET,	coords.BOX_RIGHT_W_OFFSET,	coords.BOX_RIGHT_H_OFFSET);
	self.boxTileLeft=	 CPP.CRect(coords.BOX_LEFT_X_OFFSET,	 coords.BOX_LEFT_Y_OFFSET,	 coords.BOX_LEFT_W_OFFSET,	coords.BOX_LEFT_H_OFFSET );
	self.boxTileUp=		 CPP.CRect(coords.BOX_UP_X_OFFSET,		coords.BOX_UP_Y_OFFSET,			coords.BOX_UP_W_OFFSET,			coords.BOX_UP_H_OFFSET	 );

	self.boxTileDownA= CPP.CRect(coords.BOX_GROUND_A_X_OFFSET,	coords.BOX_GROUND_Y_OFFSET,		1,	coords.BOX_GROUND_H_OFFSET);
	self.boxTileDownB= CPP.CRect(coords.BOX_GROUND_B_X_OFFSET,	coords.BOX_GROUND_Y_OFFSET,		1,	coords.BOX_GROUND_H_OFFSET);

	self.cComp:AddCollisionBox(self.boxTileDownA, self.boxID.BOX_TILE_DOWN_A, coords.BOX_GROUND_ORDER);
	self.cComp:CheckForTiles(self.boxID.BOX_TILE_DOWN_A);

	self.cComp:AddCollisionBox(self.boxTileDownB, self.boxID.BOX_TILE_DOWN_B, coords.BOX_GROUND_ORDER);
	self.cComp:CheckForTiles(self.boxID.BOX_TILE_DOWN_B);

	self.cComp:AddCollisionBox(self.boxTileRight, self.boxID.BOX_TILE_RIGHT, coords.BOX_RIGHT_ORDER);
	self.cComp:CheckForTiles(self.boxID.BOX_TILE_RIGHT);

	self.cComp:AddCollisionBox(self.boxTileLeft, self.boxID.BOX_TILE_LEFT, coords.BOX_LEFT_ORDER);
	self.cComp:CheckForTiles(self.boxID.BOX_TILE_LEFT);

	self.cComp:AddCollisionBox(self.boxTileUp, self.boxID.BOX_TILE_UP, coords.BOX_UP_ORDER);
	self.cComp:CheckForTiles(self.boxID.BOX_TILE_UP);

	self.groundTouch=false;
end

function collision.SetWidthHeight(self, w, h)
	self.WIDTH=w;
	self.HEIGHT=h;
	self.coordinates.BOX_GROUND_A_X_OFFSET =  w-2
	self.coordinates.BOX_GROUND_B_X_OFFSET =  2;
	self.coordinates.BOX_GROUND_Y_OFFSET		=  h-self.BOX_FEET_OFFSET;
	self.coordinates.BOX_GROUND_H_OFFSET		=  1+self.BOX_FEET_OFFSET;
	self.coordinates.BOX_GROUND_ORDER			=  5;

	--The left and right self boxes are closer to the top than the bottom
	--this is to allow for covering rougher terrain without the left and right
	--self boxes incorrectly setting off a self event
	--A happy side effect is that the character feet will land on the ledge at a
	--higher y coordinate (lower on the screen) than they would otherwise
	--(when such a self would result in left or right firing instead of feet)

	self.coordinates.BOX_RIGHT_X_OFFSET		=  w
	self.coordinates.BOX_RIGHT_Y_OFFSET		=  6
	self.coordinates.BOX_RIGHT_W_OFFSET		=  1
	self.coordinates.BOX_RIGHT_H_OFFSET		=  h-14;
	self.coordinates.BOX_RIGHT_ORDER				=  15;

	self.coordinates.BOX_LEFT_X_OFFSET			=  0;
	self.coordinates.BOX_LEFT_Y_OFFSET			=  6
	self.coordinates.BOX_LEFT_W_OFFSET			=  -1;
	self.coordinates.BOX_LEFT_H_OFFSET			=  h-14;
	self.coordinates.BOX_LEFT_ORDER				=  15;

	self.coordinates.BOX_UP_Y_OFFSET				=  0;
	self.coordinates.BOX_UP_H_OFFSET				=  -1;
	self.coordinates.BOX_UP_W_OFFSET				=  (w/2);
	self.coordinates.BOX_UP_X_OFFSET				=  (w/4);
	self.coordinates.BOX_UP_ORDER					=  10;
end

function collision.GetHeightMapValue(self, absoluteX, tileCollisionPacket)
	if(tileCollisionPacket.layer:UsesHMaps() == true) then return 16 end
	local box_value = 0;
	local boxid=tileCollisionPacket:GetID();
	local hmap=tileCollisionPacket:GetHmap();
	local HMAP_index_value= 0;
	local tx=tileCollisionPacket:GetX();

	--First, figure out the x-coordinate of the heightmap value (height map index value)
	if(boxid==collision.boxID.BOX_TILE_DOWN_A) then
		box_value = self.coordinates.BOX_GROUND_A_X_OFFSET;
	elseif(boxid==collision.boxID.BOX_TILE_DOWN_B) then
		box_value = self.coordinates.BOX_GROUND_B_X_OFFSET;
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
		self.footHeightValue=math.floor(yspd+0.5+self.BOX_FEET_OFFSET+math.abs(xspd))+2
	else
		self.footHeightValue=math.floor(yspd + 0.5 + self.BOX_FEET_OFFSET)+1
	end

	self.cComp:ChangeHeight(self.boxID.BOX_TILE_DOWN_A,  self.footHeightValue);
	self.cComp:ChangeHeight(self.boxID.BOX_TILE_DOWN_B,  self.footHeightValue);
	self.cComp:ChangeHeight(self.boxID.BOX_TILE_UP,		math.floor(yspd - 2.5));
	self.cComp:ChangeWidth(self.boxID.BOX_TILE_LEFT,		math.floor(xspd - 0.5));
	self.cComp:ChangeWidth(self.boxID.BOX_TILE_RIGHT,		math.floor(xspd + 0.5));

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
	if ( ((boxid==self.boxID.BOX_TILE_DOWN_A) or (boxid==self.boxID.BOX_TILE_DOWN_B))
		 and(tx~=self.ignoreTileX)
		 ) then

		local thisAngle=hmap.angleH;
		local thisAngleSigned= self.AngleToSignedAngle(thisAngle);
		local maximumFootY=self.footHeightValue + exactY + self.coordinates.BOX_GROUND_Y_OFFSET;
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
	elseif ( (boxid==self.boxID.BOX_TILE_RIGHT) ) then
		if(usesHMaps)then
			return;
		end
		if((hspd>=0) or ((self.groundTouch==false) and (hspd==0)))  then
			--Add one because the first x pixel counts as part of the width
			newPosition=CPP.Coord2df( (tx*self.TILEWIDTH)-self.coordinates.BOX_RIGHT_X_OFFSET-self.coordinates.BOX_RIGHT_W_OFFSET+1,																																																			yval);

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
	elseif ( (boxid==self.boxID.BOX_TILE_LEFT) ) then
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
	elseif ( (boxid==self.boxID.BOX_TILE_UP) and(tx~=self.ignoreTileX))then
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
