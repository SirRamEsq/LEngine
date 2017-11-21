--- ImGui TestBed
-- Use this for a reference as to how to use the CPP.ImGui api


local result=0;
result, imGuiFlags = pcall(loadfile(utilityPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewMain(baseclass)
	local main = baseclass or {}

	function main.Initialize()
		main.depth		= main.LEngineData.depth;
		main.parent		= main.LEngineData.parent;
		main.EID		= main.LEngineData.entityID;

		main.defaultWindowPos = CPP.Coord2df(450, 300)
		local guiName = ""
		local guiScript = "GUI/guiTest.lua"
		main.childEID = CPP.interface:EntityNew(guiScript, 0,0, main.depth, main.EID, guiScript, {})
		--main.font = "extra_fonts/Roboto-Medium.ttf"
		main.font = "ebFonts/wisdom.ttf"
		main.fontSize = 30
		main.pushedState = nil
		main.winName = "MAIN"

		main.SetData("t", {"Success!", 5})
	end

	function main.Update()
		local windowFlags = imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove + imGuiFlags.AlwaysAutoResize
		local resolution = CPP.interface:GetResolution()

		local popFont = CPP.ImGui.PushFont(main.font, main.fontSize)
		--CPP.ImGui.SetNextWindowPos(main.defaultWindowPos, 0)
		--CPP.ImGui.SetNextWindowPosCenter(0)
		CPP.ImGui.SetNextWindowFocus();

		CPP.ImGui.BeginFlags(main.winName, windowFlags)

		if(CPP.ImGui.Button("Proceed"))then
			main.pushedState = CPP.interface:PushState("States/levelState.lua")
		end
		CPP.ImGui.SameLine()
		if(CPP.ImGui.Button("Back"))then
			CPP.interface:PopState()
		end

		CPP.ImGui.Text("DATA:  " .. tostring(main.GetData("t")[1]))
		--Should be nil
		CPP.ImGui.Text("DATA2: " .. tostring(main.data))
		CPP.ImGui.Separator()	
		local time = os.time() 
		CPP.ImGui.Text("Date:  " .. tostring(os.date("%x", time)))
		CPP.ImGui.Text("Time:  " .. tostring(os.date("%I:%M:%S", time)))

		main.winSize = CPP.ImGui.GetWindowSize()
		CPP.ImGui.End()

		--Center Window
		main.currentPosition = CPP.Coord2df(( resolution.x/2) - (main.winSize.x/2), 0)

		CPP.ImGui.SetWindowPos(main.winName, main.currentPosition, 0)

		if(popFont)then
			CPP.ImGui.PopFont()
		end
	end

	return main;
end

return container.NewMain;
