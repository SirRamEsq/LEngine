require 'busted.runner'()
require "lfs"

local result=0;
CPP = {}
result, CPP = pcall(loadfile(lfs.currentdir () .. "/Mocks/mock_cpp.lua"))

local newENV = _ENV
newENV.CPP=CPP

local input;
result, input = pcall(loadfile(lfs.currentdir () .. "/../Common/input.lua", "t", newENV))

describe("Input Test", function()
	it("Can pass sanity test", function()
		assert.is_nil(nil)
		assert.is_table(input)
	end)

	--register keys
	local upKey = "up"
	local downKey = "down"
	local leftKey = "left"
	local rightKey = "right"
	input.RegisterKey(upKey)
	input.RegisterKey(downKey)
	input.RegisterKey(leftKey)
	input.RegisterKey(rightKey)

	it("Registers KeyPresses", function()
		assert.is_false(input.key[upKey])
		assert.is_false(input.key[downKey])
		assert.is_false(input.key[leftKey])
		assert.is_false(input.key[rightKey])

		input.OnKeyDown(upKey);
		input.Update()

		assert.is_true(input.key[upKey])
		assert.is_false(input.key[downKey])
		assert.is_false(input.key[leftKey])
		assert.is_false(input.key[rightKey])

		assert.is_true(input.keyPress[upKey])
		assert.is_false(input.keyPress[downKey])
		assert.is_false(input.keyPress[leftKey])
		assert.is_false(input.keyPress[rightKey])

		assert.is_false(input.keyRelease[upKey])
		assert.is_false(input.keyRelease[downKey])
		assert.is_false(input.keyRelease[leftKey])
		assert.is_false(input.keyRelease[rightKey])

		input.OnKeyUp(upKey)
		input.Update()

		assert.is_true(input.keyRelease[upKey])
		assert.is_false(input.key[upKey])
		assert.is_false(input.keyPress[upKey])
	end)
end)
