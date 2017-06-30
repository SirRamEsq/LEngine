require 'busted.runner'()
require "lfs"

local result=0;
CPP = {}
result, CPP = pcall(loadfile(lfs.currentdir () .. "/Mocks/mock_cpp.lua"))

local newENV = _ENV
newENV.CPP=CPP

local collision;
result, collision = pcall(loadfile(lfs.currentdir () .. "/../Common/entityCollisionSystemNew.lua", "t", newENV))

local col
result, col = pcall(loadfile(lfs.currentdir () .. "/Mocks/mock_collisionComponent.lua", "t", newENV))

describe("a test", function()
	--tests
	it("checks thing", function()
		assert.is_nil(nil)
		assert.is_table(collision)
	end)

	it("Startup collision", function()
		collision:Init(32, 32, col, 10)
	end)
end)
