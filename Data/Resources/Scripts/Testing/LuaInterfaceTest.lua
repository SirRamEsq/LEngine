function NEWTESTCLASS(baseclass)
	local test = baseclass or {}

	function test.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------

		test.depth        = test.LEngineData.depth;
		test.parent       = test.LEngineData.parent;
		test.CPPInterface = CPP.interface; 
		test.EID          = test.LEngineData.entityID;

		if(test.CPPInterface ~= nil) then CPP.interface:WriteError(test.EID, "Good") end
	end
	
	return test
end

return NEWTESTCLASS
