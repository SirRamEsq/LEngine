local result=0;

local container = {}
function container.NewState(baseclass)
	local state = baseclass or {}


	--Run at instantiation
	function state.Initialize()
		state.depth		= state.LEngineData.depth;
		state.parent	= state.LEngineData.parent;
		state.EID		= state.LEngineData.entityID;
		state.name = "NAME"

		local posX = 16
		local posY = 16
		local depth = 100
		local name = state.name
		local type =""

		--Create Entities
		state.entity1 = CPP.interface:EntityNew( "dummy.lua", posX, posY, depth,  0, name, type, {} )
		state.entity2 = CPP.interface:EntityNew( "dummy.lua", posX, posY, depth,  0, name, type, {} )
		state.entity3 = CPP.interface:EntityNew( "dummy.lua", posX, posY, depth,  0, name, type, {} )
	end

	--Run Before every test
	function state.Setup(testing)
		testing:REQUIRE_NOT_EQUAL(state.entity1, 0)
		testing:REQUIRE_NOT_EQUAL(state.entity2, 0)
		testing:REQUIRE_NOT_EQUAL(state.entity3, 0)
	end

	function state.LookupEntitiesFromName(testing)
		local entities = CPP.interface:EntityGetFromName(state.name)
		local success = testing:REQUIRE_NOT_EQUAL(entities, nil)
		if not success then return end

		local count = 0
		for i=0, entities:size() - 1 do
			count = count + 1
			local v = entities:at(i)
			if (state.entity1 ~= v) and (state.entity2 ~= v) and (state.entity3 ~= v) then
				testing:Error("Returned EID '" .. tostring(v) .. "' does not match any created")
			end
		end

		testing:REQUIRE_EQUAL(count, 3)
	end
	--Run after every Test
	function state.Teardown(testing)

	end

	state.TESTS = {state.LookupEntitiesFromName}

	return state;
end

return container.NewState;
