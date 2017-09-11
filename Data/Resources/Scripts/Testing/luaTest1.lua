require 'busted.runner'()
require "lfs"

local result=0;
CPP = {}
result, CPP = pcall(loadfile(lfs.currentdir () .. "/Mocks/mock_cpp.lua"))

local newENV = _ENV
newENV.CPP=CPP

local collision;
result, collision = pcall(loadfile(lfs.currentdir () .. "/../Common/entityCollisionSystemNew.lua", "t", newENV))

local tileCollision;
result, tileCollision = pcall(loadfile(lfs.currentdir () .. "/../Common/tileCollisionSystemNew.lua", "t", newENV))

local packetGenerator;
packetGenerator= loadfile(lfs.currentdir () .. "/Mocks/mock_tileCollisionPacket.lua", "t", newENV)
local hmapGenerator;
hmapGenerator= loadfile(lfs.currentdir () .. "/Mocks/mock_hmap.lua", "t", newENV)
local layerGenerator;
layerGenerator= loadfile(lfs.currentdir () .. "/Mocks/mock_tileLayer.lua", "t", newENV)

local col
result, col = pcall(loadfile(lfs.currentdir () .. "/Mocks/mock_collisionComponent.lua", "t", newENV))

local eid = 10

describe("Entity Collision Test", function()
	--tests
	it("can pass sanity test", function()
		assert.is_nil(nil)
		assert.is_table(collision)
	end)

	it("Can Initialize and Set Width / Height", function()
		local w = 32
		local h = 32
		collision:Init(w, h, col, eid)
		assert.is_equal(w,collision.WIDTH)
		assert.is_equal(h,collision.HEIGHT)

		w = 16
		h = 16
		collision:SetWidthHeight(w,h)
		assert.is_equal(w,collision.WIDTH)
		assert.is_equal(h,collision.HEIGHT)
	end)
end)

describe("Tile Collision Basic Test", function()
	it("Can pass sanity test", function()
		assert.is_nil(nil)
		assert.is_table(tileCollision)
	end)

	it("Can Initialize and Set Width / Height", function()
		local w = 32
		local h = 32
		tileCollision:Init(w, h, CPP, col, eid)
		assert.is_equal(w,tileCollision.WIDTH)
		assert.is_equal(h,tileCollision.HEIGHT)

		w = 16
		h = 16
		tileCollision:SetWidthHeight(w,h)
		assert.is_equal(w,tileCollision.WIDTH)
		assert.is_equal(h,tileCollision.HEIGHT)
	end)

	it("Updates resize boxes correctly", function()
		local xspd = 0
		local yspd = 5

		--yspd is moving downward, t>herefore the box should be longer after updating
		local previousBoxHeight = col:GetBox(tileCollision.boxID.TILE_DOWN_R).h
		local previousBoxWidth = col:GetBox(tileCollision.boxID.TILE_LEFT).w
		tileCollision:Update(xspd, yspd)
		local newBoxHeight = col:GetBox(tileCollision.boxID.TILE_DOWN_R).h
		local newBoxWidth = col:GetBox(tileCollision.boxID.TILE_LEFT).w
		assert.is_true(newBoxHeight > previousBoxHeight)
		assert.is_equal(newBoxWidth, previousBoxWidth)

		xspd = 5
		yspd = 0

		--yspd is now zero, box should be shorter than previous frame
		--xspd is moving right, left box should extend further to the right
		--(therefore having a larget width)
		previousBoxHeight = col:GetBox(tileCollision.boxID.TILE_DOWN_R).h
		previousBoxWidth = col:GetBox(tileCollision.boxID.TILE_LEFT).w
		tileCollision:Update(xspd, yspd)
		newBoxHeight = col:GetBox(tileCollision.boxID.TILE_DOWN_R).h
		newBoxWidth = col:GetBox(tileCollision.boxID.TILE_LEFT).w
		assert.is_true(newBoxHeight < previousBoxHeight)
		assert.is_true(newBoxWidth > previousBoxWidth)
	end)

	it("Run Collision Function", function()
		local tilePacket
		local result
		result, tilePacket = pcall(packetGenerator)
		if(result == false) then error(tilePacket) end
		
		local hmap
		result, hmap = pcall(hmapGenerator)
		if(result == false) then error(hmap) end

		local layer
		result, layer = pcall(layerGenerator)
		if(result == false) then error(layer) end
		tilePacket.hmap = hmap
		tilePacket.layer = layer

		local xspd = 0
		local yspd = 1
		local exactX = 24
		local exactY = 31

		tileCollision:OnTileCollision(tilePacket, xspd, yspd, exactX, exactY)
	end)
end)

function callbackDummy()
	local timesCalled = 0
	local arguments = {}
	local object = {}
		
	function object.Reset()
		timesCalled=0
		arguments={}
	end

	function object.GetStatus()
		return timesCalled, arguments
	end

	return function(newPosition, angle)
		arguments[timesCalled]=newPosition
		timesCalled = timesCalled + 1
	end
	,
	object
end
describe("Tile Collision detection", function()
	--Responsibility of the tile collision modeul is to do two things
	--Set appropriate sizes of collision boxes based on entity speed
	--React to collision events with tile appropriately
	--It is NOT the job of this module to determine when a collision has occured
	setup(function()
		globalTilePacket = nil
		local result
		result, globalTilePacket = pcall(packetGenerator)
		if(result == false) then error(globalTilePacket) end
		
		local hmap
		result, hmap = pcall(hmapGenerator)
		if(result == false) then error(hmap) end

		local layer
		result, layer = pcall(layerGenerator)
		if(result == false) then error(layer) end
		globalTilePacket.hmap = hmap
		globalTilePacket.layer = layer

		local w = 16
		local h = 16
		tileCollision:Init(w, h, CPP, col, eid)
	end)

	it("Getting HeightMap Value works", function()
		local tileX = 5
		local tileY = 5
		local tileW = 16
		local tileH = 16

		globalTilePacket.layer.tiles[tileX]={}
		globalTilePacket.layer.tiles[tileX][tileY] = 20 --set tile x=5 y=5 to a non-zero value indicating it's solid
		globalTilePacket.id = tileCollision.boxID.TILE_DOWN_L
		local boxOffset = tileCollision.coordinates.GROUND_L_X_OFFSET
		globalTilePacket.x = tileX
		globalTilePacket.y = tileY
		local absolutex = (tileX * tileW) + 15 - boxOffset

		tileCollision:GetHeightMapValue(absolutex, globalTilePacket)
	end)

	it("Registers collisions when they should obviously work", function()
		--no movement, x,y is at the very top right corner of the specified tile
		--the collision box should extend to the right and downward by its width and height respectively
		--in this case, the collision box should overlap the tile and should register a collision

		local xspd = 0 
		local yspd = 0
		local tileX = 5
		local tileY = 5
		local tileW = 16
		local tileH = 16
		local entityW = 16
		local entityH = 16

		tileCollision:Init(entityW, entityH, CPP, col, eid)

		globalTilePacket.id = tileCollision.boxID.TILE_DOWN_L
		globalTilePacket.x = tileX
		globalTilePacket.y = tileY
		local tileDownCallback
		local callbackStatus
		tileDownCallback, callbackStatus= callbackDummy()	
		tileCollision.callbackFunctions.TileDown = tileDownCallback
		tileCollision:Update(xspd, yspd)
		tileCollision:OnTileCollision(globalTilePacket, xspd, yspd, (tileX * tileW), (tileY * tileH)) 

		local args
		local timesCalled
		timesCalled, args = callbackStatus.GetStatus()

		assert.is_equal(1, timesCalled)
		callbackStatus.Reset()
	end)
end)