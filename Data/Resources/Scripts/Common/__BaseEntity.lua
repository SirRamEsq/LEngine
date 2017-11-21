--Base class for ALL Entities run from LEngine

local BaseEntity = {}

function BaseEntity.new(ignoringThis)
	--http://lua-users.org/wiki/ObjectOrientationClosureApproach
	--baseclass can be passed, but is completely ignored
	local base = {}

	--Constant variables, meant to be set once by other scripts
	base.C = {}
	base.C.WIDTH = 0
	base.C.HEIGHT = 0
	base.UpdateFunctions = {}
	base.InitFunctions = {}
	base.LuaEventFunctions = {}

	function base.Initialize()
		for k,v in pairs(base.InitFunctions) do
			v()
		end
	end

	function base.Update()
		for k,v in pairs(base.UpdateFunctions) do
			v()
		end
	end

	function base.Attacked(damage)
		return false;
	end

	function base.OnLuaEvent(eid, description)
		for k,v in pairs(base.LuaEventFunctions) do
			v(eid, description)
		end
	end

	function base.OnKeyUp(keyname)

	end

	function base.OnKeyDown(keyname)

	end

	base.EntityInterface = {
		Activate      = function () end,
		IsSolid		  = function () return true;  end,
		IsCollectable = function () return 0;  end,
		CanBounce     = function () return false; end, --the 'goomba' property
		CanGrab		  = function () return false; end,
		--This instance being attacked, returns true if attack hit, false if not
		Attack        = function (damage) return base.Attacked(damage); end,  --this instance being attacked
		Land          = function () return 1; end--This instance being landed on
	}

	return base;
end

return BaseEntity.new
