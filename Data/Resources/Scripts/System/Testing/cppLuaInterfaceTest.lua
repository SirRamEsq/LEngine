local cppTest = {}

cppTest.create = function (baseclass)
	local test = baseclass or {}
	test = {}

	function test.Initialize()
		test.depth        = test.LEngineData.depth;
		test.EID          = test.LEngineData.entityID;
		test.stateEID          = test.LEngineData.stateEID;
		test.correctKeyPress = "up"
		test.incorrectKeyPress = "down"
		local cpp = CPP.interface

		if(cpp ~= nil) then
			cpp:LogError(test.EID, "Interface Working")
		else
			error ("CPP.interface is NIL!")
		end

		cpp:ListenForInput(test.EID, test.correctKeyPress)
		cpp:ListenForInput(test.EID, test.incorrectKeyPress)
	end

	function test.OnKeyDown(keyname)
		if (keyname == test.correctKeyPress)then
			CPP.interface:LogError(test.EID, "KeyPress: Correct Input")
		else
			CPP.interface:LogError(test.EID, "KeyPress: Incorrect Input")
		end
	end

	function test.OnKeyUp(keyname)
		if (keyname == test.correctKeyPress)then
			CPP.interface:LogError(test.EID, "KeyRelease: Correct Input")
		else
			CPP.interface:LogError(test.EID, "KeyRelease: Incorrect Input")
		end
	end

	function test.OnLuaEvent(senderEID, eventString)
		CPP.interface:LogError(test.EID, "EVENT: " .. eventString)
	end

	function test.Observe31337()
		local cScript = CPP.interface:GetScriptComponent(31337)
		cScript:AddObserver(test.EID)
		CPP.interface:LogError(test.EID, "Observing 31337")
	end

	function test.PrintEID()
		CPP.interface:LogError(test.EID, tostring(test.EID))
	end

	function test.PrintStateEID()
		CPP.interface:LogError(test.EID, tostring(test.stateEID))
	end

	return test
end

return cppTest.create
