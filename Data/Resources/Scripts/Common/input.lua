--[[
--Module will return a container that will handle input
--Simply redirect keyPress / KeyRelease events to its respective functions
--and Update accordingly
--
--To access, simply do:
--	To see if the key is held down
--	input.key.name
--
--	To see if key is pressed
--	input.keyPress.name
--
--	To see if key is released this Frame
--	input.keyRelease.name
--]]

local input = {}

input.key			= {}
input.keyPress		= {}
input.keyRelease	= {}
input.keyPrevious	= {}

function input.RegisterKey(keyName)
	input.key[keyName] = false
	input.keyPress[keyName] = false
	input.keyRelease[keyName] = false
	input.keyPrevious[keyName] = false
end

function input.OnKeyDown(keyName)
	local key = input.key[keyName]
	if(key ~= nil)then
		input.key[keyName] = true
	end
end

function input.OnKeyUp(keyName)
	local key = input.key[keyName]
	if(key ~= nil)then
		input.key[keyName] = false
	end
end

function input.Update()
	for k,v in pairs(input.key) do
		--If the key is being pressed and wasn't previously, then keyPress is true
		if		((v==true)and(input.keyPrevious[k]==false))then
			input.keyPress[k]=true;
			input.keyRelease[k]=false;

		--If the key is not being pressed and it was previously, then keyRelease is true
		elseif	((v==false)and(input.keyPrevious[k]==true))then
			input.keyPress[k]=false;
			input.keyRelease[k]=true;

		else
			input.keyPress[k]=false;
			input.keyRelease[k]=false;

		end
		input.keyPrevious[k]=v;
	end
end

return input
