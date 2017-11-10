local cppTest = {}

cppTest.create = function (baseclass)
	local test = baseclass or {}
	test = {}

	function test.Initialize()
		test.depth        = test.LEngineData.depth;
		test.parentEID       = test.LEngineData.parentEID;
		test.CPPInterface = CPP.interface; 
		test.name=			test.LEngineData.name
		test.objType=test.LEngineData.objType
		test.EID          = test.LEngineData.entityID;
		test.correctKeyPress = "up"
		test.incorrectKeyPress = "down"

		if(test.CPPInterface ~= nil) then
			CPP.interface:LogError(test.EID, "Interface Working")
		else
			error ("CPP.interface is NIL!")
		end

		test.CPPInterface:ListenForInput(test.EID, test.correctKeyPress)
		test.CPPInterface:ListenForInput(test.EID, test.incorrectKeyPress)
	end

	function test.OnKeyDown(keyname)
		if (keyname == test.correctKeyPress)then
			test.CPPInterface:LogError(test.EID, "KeyPress: Correct Input")
		else
			test.CPPInterface:LogError(test.EID, "KeyPress: Incorrect Input")
		end
	end

	function test.OnKeyUp(keyname)
		if (keyname == test.correctKeyPress)then
			test.CPPInterface:LogError(test.EID, "KeyRelease: Correct Input")
		else
			test.CPPInterface:LogError(test.EID, "KeyRelease: Incorrect Input")
		end
	end

	function test.OnLuaEvent(senderEID, eventString)
		test.CPPInterface:LogError(test.EID, "EVENT: " .. eventString)
	end

	function test.Observe31337()
		test.CPPInterface:EventLuaObserveEntity(test.EID, 31337)
		test.CPPInterface:LogError(test.EID, "Observing 31337")
	end

	function test.PrintDepth()
		CPP.interface:LogError(test.EID, tostring(test.depth))
	end
	function test.PrintParent()
		CPP.interface:LogError(test.EID, tostring(test.parentEID))
	end
	function test.PrintEID()
		CPP.interface:LogError(test.EID, tostring(test.EID))
	end
	function test.PrintName()
		CPP.interface:LogError(test.EID, tostring(test.name))
	end
	function test.PrintType()
		CPP.interface:LogError(test.EID, tostring(test.objType))
	end

	return test
end

return cppTest.create
