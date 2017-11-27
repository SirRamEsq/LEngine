--returns table

local fade = {}

fade.FadeOut = function(layer, alphaDecrement)
	if (layer:GetAlpha() <= 0)then return nil end
	local fadeOut = {}
	fadeOut.decrement = alphaDecrement or 0.01
	fadeOut.layer = layer
	fadeOut.initAlpha = layer:GetAlpha()
	fadeOut.alpha = fadeOut.initAlpha

	--[[
	--meant to be used like this:
	--fade = import fadeOut
	--local fadeOut = fade.FadeOut(local)
	--
	--...
	--
	--if(fadeOut ~= nil)then
	--    fadeOut = fadeOut()
	--end
	--]]
	fadeOut.Update = function()
		if(fadeOut.alpha == 0) then
			--CPP.interface:DeleteLayer(fadeOut.layer)
			return nil
		end

		fadeOut.alpha = fadeOut.alpha - fadeOut.decrement
		if(fadeOut.alpha < 0) then fadeOut.alpha = 0 end

		fadeOut.layer:SetAlpha(fadeOut.alpha)
		return fadeOut.Update
	end

	return fadeOut.Update
end

return fade
