
--Base class for ALL Entities run from LEngine

local EmeraldPiece = {}

function EmeraldPiece.new(base)
	local emerald = base or {}

	--Constant variables, meant to be set once
	emerald.C = {}
	emerald.C.WIDTH  = 0
	emerald.C.HEIGHT = 0

	emerald.xspd = 0
	emerald.yspd = 0

	function emerald.Attacked(damage)
		return false;
	end

	function emerald.OnLuaEvent(eid, description)

	end

	return emerald;
end

return EmeraldPiece.new
