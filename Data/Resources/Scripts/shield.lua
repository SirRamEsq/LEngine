--Constants
	TILEWIDTH=16;
	TILEHEIGHT=16;
	
	MAXSPEED=15;
	
	WIDTH=16; --WIDTH of object
	HEIGHT=16; --HEIGHT of object
	
	DEACCELERATION=.046875;
	STARTSPEED=12;
	
	FRICTION_AIR=.2;
	FRICTION_MODIFER=.046875; --The friction of a tile is multiplied by this constant to get the actual friction value
	
--Variables
	--General Movement Variables
		xspd=0;
		yspd=0;
	
	--C++ Interfacing
		CPPInterface=nil;
		mySprite=nil;
		mySpriteComp=nil;
		myColComp=nil;
		EID=0;
		depth=0;
		parent=0;

--Collision boxes and ID Numbers
		--PrimaryBox
		CBOX_PRIME=0;
		
		CBOX_TILE_UP=5;
		CBOX_TILE_LEFT=6;
		CBOX_TILE_RIGHT=7;
		CBOX_TILE_DOWN_A=8;
		CBOX_TILE_DOWN_B=9;

--Collision Boxes
		--Entity Collision Boxes
			cboxPrimary=nil;
			
		--Tile Collision Boxes
			cboxTileRight=nil;
			cboxTileLeft=nil;
			cboxTileUp=nil;
			cboxTileDown=nil;
	
function Init(LuaInterface, DEPTH, par, extraParams)
	--------------------
	--LUA Verion Check--
	--------------------
	if (_VERSION == "Lua 5.1") then
		LUA51=true;
	else
		LUA51=false;
	end
	
	LuaInterface:WriteError("it is: " .. tostring(extraParams));
	LuaInterface:WriteError(type(extraParams));
	
	for key,value in pairs(extraParams) do LuaInterface:WriteError(tostring(key) .. tostring(value)) end

	-----------------------
	--C++ Interface setup--
	-----------------------
	
	depth=DEPTH;
	parent=par;
	CPPInterface=LuaInterface;
	EID=CPPInterface:GetEID();
	
	CPPInterface:WriteError("Parent is: " .. parent);
	
	CPPInterface:AddSpriteComponent();
	CPPInterface:AddCollisionComponent();
	myColComp=CPPInterface:GetCollisionComp();
	mySpriteComp=CPPInterface:GetSpriteComp();
	
	----------------
	--Sprite setup--
	----------------
	
	mySpriteComp:SetRotation(0);
	mySprite=CPPInterface:LoadSprite("ShieldSPR");
	if(mySprite==nil) then
		CPPInterface:WriteError("sprite is NIL");
	else
		mySprite:AddAnimation("Shield");

		aniRect=CRect(0,0,WIDTH,HEIGHT);
	
		for i=0, 3 do
			mySprite:AppendImage("Shield",aniRect,"Shield.png");
			aniRect.x= aniRect.x + WIDTH;
		end
		
		mySpriteComp:SetSprite(mySprite);
		mySprite=mySpriteComp:GetSprite();
		mySprite:SetColorKey("Shield",0,255,255,255);
		mySprite:SetCurrentAnimation("Shield");
		mySprite:SetSpeed(.5);
	end
	
	mySpriteComp:SetDepth(depth);
	
	-----------------------
	--Collision for tiles--
	-----------------------
	local buffer;
	buffer=0;
	
	cboxTileRight=	CRect(WIDTH-buffer,	20,	(WIDTH/2)+buffer,	0);
	cboxTileLeft=	CRect(0+buffer,       	20, 	-(WIDTH/2)-buffer, 	0);
	cboxTileUp=		CRect(WIDTH/2, 		0+buffer,	0,			-(HEIGHT/2)-buffer);
	
	cboxTileDownA=	CRect(WIDTH-10,	HEIGHT, 	0,		 HEIGHT--[[(HEIGHT/2)+(HEIGHT/8) +7)--]]);
	cboxTileDownB=	CRect(10,	HEIGHT, 	0,		 HEIGHT--[[(HEIGHT/2)+(HEIGHT/8) +7)--]]);
	
	cboxPrimary=	CRect(5,	5, 		WIDTH-10,	HEIGHT-10);

	--Setup the boxes for when bobMode==MODE_DOWN
	
	myColComp:AddCollisionBox(cboxTileDownA, CBOX_TILE_DOWN_A, 0);
	myColComp:CheckForTiles(CBOX_TILE_DOWN_A);
	
	myColComp:AddCollisionBox(cboxTileDownB, CBOX_TILE_DOWN_B, 0);
	myColComp:CheckForTiles(CBOX_TILE_DOWN_B);
	
	myColComp:AddCollisionBox(cboxTileRight, CBOX_TILE_RIGHT, 0);
	myColComp:CheckForTiles(CBOX_TILE_RIGHT);
	
	myColComp:AddCollisionBox(cboxTileLeft, CBOX_TILE_LEFT, 0);
	myColComp:CheckForTiles(CBOX_TILE_LEFT);
	
	myColComp:AddCollisionBox(cboxTileUp, CBOX_TILE_UP, 0);
	myColComp:CheckForTiles(CBOX_TILE_UP);
	
	myColComp:AddCollisionBox(cboxPrimary, CBOX_PRIME, 0);
	myColComp:CheckForEntities(CBOX_PRIME);
	myColComp:SetPrimaryCollisionBox(0, false);
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

function Update()
	
	
end

function OnEntityCollision(entityID, myBoxID)
	
end

function OnTileCollision(packet)
	local boxid=packet:GetID();
	local tx=packet:GetX();
	local ty=packet:GetY();
	
	--Actual TileLayer Structure
	local layer=packet:GetLayer();
	
	if ( ((boxid==CBOX_TILE_DOWN_A) or (boxid==CBOX_TILE_DOWN_B)) and (yspd>0) ) then
		CPPInterface:MovePosAbsY((ty*16) - HEIGHT);
		groundTouch=true;
	end
end