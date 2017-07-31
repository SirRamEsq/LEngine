local cppTest = {}

cppTest.create = function (baseclass)
	local test = baseclass or {}
	test = {}

	function test.Initialize()
		test.depth        = test.LEngineData.depth;
		test.parent       = test.LEngineData.parent;
		test.CPPInterface = CPP.interface; 
		test.name=			test.LEngineData.name
		test.objType=test.LEngineData.objType
		test.EID          = test.LEngineData.entityID;
		test.correctKeyPress = "up"
		test.incorrectKeyPress = "down"

		if(test.CPPInterface ~= nil) then
			CPP.interface:WriteError(test.EID, "Interface Working")
		else
			error ("CPP.interface is NIL!")
		end

		test.CPPInterface:ListenForInput(test.EID, test.correctKeyPress)
		test.CPPInterface:ListenForInput(test.EID, test.incorrectKeyPress)
	end

	function test.OnKeyDown(keyname)
		if (keyname == test.correctKeyPress)then
			test.CPPInterface:WriteError(test.EID, "KeyPress: Correct Input")
		else
			test.CPPInterface:WriteError(test.EID, "KeyPress: Incorrect Input")
		end
	end

	function test.OnKeyUp(keyname)
		if (keyname == test.correctKeyPress)then
			test.CPPInterface:WriteError(test.EID, "KeyRelease: Correct Input")
		else
			test.CPPInterface:WriteError(test.EID, "KeyRelease: Incorrect Input")
		end
	end

	function test.OnLuaEvent(senderEID, eventString)
		test.CPPInterface:WriteError(test.EID, "EVENT: " .. eventString)
	end

	function test.Observe31337()
		test.CPPInterface:EventLuaObserveEntity(test.EID, 31337)
		test.CPPInterface:WriteError(test.EID, "Observing 31337")
	end

	function test.PrintDepth()
		CPP.interface:WriteError(test.EID, tostring(test.depth))
	end
	function test.PrintParent()
		CPP.interface:WriteError(test.EID, tostring(test.parent))
	end
	function test.PrintEID()
		CPP.interface:WriteError(test.EID, tostring(test.EID))
	end
	function test.PrintName()
		CPP.interface:WriteError(test.EID, tostring(test.name))
	end
	function test.PrintType()
		CPP.interface:WriteError(test.EID, tostring(test.objType))
	end

	return test
end

return cppTest.create
