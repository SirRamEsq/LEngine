--[[
--TO USE:
--C.WIDTH and C.HEIGHT Should be set before calling Init
--Can specify a property 'fadeOutProperty' to use as the property to search for
--to find secret layers
--
--]]
local result=0;
local fadeOut = {}
local functions = {}
result, fadeOut = pcall(loadfile(utilityPath .. "/fadeOutLayer.lua", _ENV))
result, functions = pcall(loadfile(utilityPath .. "/commonFunctions.lua", _ENV))

local container = {}
function container.NewFadeOut(baseclass)
	local class = baseclass or {}

	class.fadeSecretLayers = {}
	class.fadeSecretLayers.fadingLayers = {}

	--laconic access
	local t = class.fadeSecretLayers

	function t.OnSecretLayerTouch(packet)
		local layer = packet:GetLayer()
		--if layer has not already been added
		if(t.fadingLayers[layer] == nil)then
			t.fadingLayers[layer] = fadeOut.FadeOut(layer)
		end
	end

	function t.SecretInit()
		local initData = class.LEngineData.InitializationTable
		local EID = class.LEngineData.entityID
		local secretProperty = initData.fadeOutProperty or "isSecret"

		local collision = CPP.interface:GetCollisionComponent(EID)
		local x = class.C.WIDTH / 2
		local y = class.C.HEIGHT / 2

		t.currentMap = CPP.interface:GetMap()
		local secretLayers = CPP.interface:GetLayersWithProperty(secretProperty, true)
		if(secretLayers ~= nil) then
			if(secretLayers:empty() == false)then
				local box = CPP.Rect(x,y,1,1)
				--Create collision box only when secret layers exist
				t.boxID = collision:AddCollisionBox(box, 240);
				collision:CheckForTiles(t.boxID);

				local layerCount = secretLayers:size()
				for i=0, layerCount-1 do
					local secretLayer = secretLayers:at(i)
					if(secretLayer ~= nil)then
						collision:CheckForLayer(t.boxID, secretLayer, t.OnSecretLayerTouch)
					end
				end
			end
		end
	end

	function t.SecretUpdate()
		for k,v in pairs(t.fadingLayers) do
			v = v()
		end
	end

	table.insert(class.InitFunctions, t.SecretInit)
	table.insert(class.UpdateFunctions, t.SecretUpdate)

	return class
end

return container.NewFadeOut
