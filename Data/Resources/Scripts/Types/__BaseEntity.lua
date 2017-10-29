--Base class for ALL Entities run from LEngine

local BaseEntity = {}

function BaseEntity.new(ignoringThis)
	--http://lua-users.org/wiki/ObjectOrientationClosureApproach
	--baseclass can be passed, but is completely ignored
	local base = {}

	--Constant variables, meant to be set once
	base.C = {}
	base.C.WIDTH  = 0
	base.C.HEIGHT = 0

	base.xspd = 0
	base.yspd = 0

	function base.Attacked(damage)
		return false;
	end

	function base.OnLuaEvent(eid, description)

	end

	base.EntityInterface = {
	  Activate	  = function () end,
	  IsSolid	  = function () return true;  end,
	  CanBounce   = function () return false; end, --the 'goomba' property
	  CanClimb	  = function () return false; end, --rope
	  CanGrab	  = function () return false; end,
	  --This instance being attacked, returns true if attack hit, false if not
	  Attack	  = function (damage) return base.Attacked(damage); end,  --this instance being attacked
	  Land		  = function () return 1; end--This instance being landed on
	}

	return base;
end

return BaseEntity.new
