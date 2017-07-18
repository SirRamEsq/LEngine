local cppTEST = {}

function cppTEST.NEWTESTCLASS(baseclass)
	local test = baseclass or {}

	function test.Initialize()
		test.depth        = test.LEngineData.depth;
		test.parent       = test.LEngineData.parent;
		test.CPPInterface = CPP.interface; 
		test.EID          = test.LEngineData.entityID;
		test.correctKeyPress = "up"
		test.incorrectKeyPress = "down"

		if(test.CPPInterface ~= nil) then CPP.interface:WriteError(test.EID, "Interface Working") end

		test.CPPInterface:ListenForInput(test.EID, test.correctKeyPress)
		test.CPPInterface:ListenForInput(test.EID, test.incorrectKeyPress)
	end

	function test.OnKeyDown(keyname)
		if (keyname == test.correctKeyPress)then
			test.CPPInterface:WriteError(test.EID, "Correct Input")
		else
			test.CPPInterface:WriteError(test.EID, "Incorrect Input")
		end
	end

	function test.OnKeyUp(keyname)

	end
	
	return test
end

return cppTEST.NEWTESTCLASS
