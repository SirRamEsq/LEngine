--- ImGui TestBed
-- Use this for a reference as to how to use the CPP.ImGui api


local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewMain(baseclass)
	--Ignore baseclass
	local main = {}

	function main.Initialize()
		main.depth		= main.LEngineData.depth;
		main.parent		= main.LEngineData.parent;
		main.EID		= main.LEngineData.entityID;

		main.defaultWindowPos = CPP.Coord2df(450, 300)
		main.childEID = CPP.interface:EntityNew("GUI/guiTest.lua", 0,0, main.depth, main.EID, "NAME", "TYPE")
		--main.font = "extra_fonts/Roboto-Medium.ttf"
		main.font = "ebFonts/wisdom.ttf"
		main.fontSize = 20
	end

	function main.Update()
		local windowFlags = imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove + imGuiFlags.AlwaysAutoResize

		local popFont = CPP.ImGui.PushFont(main.font, main.fontSize)
		--CPP.ImGui.SetNextWindowPos(main.defaultWindowPos, 0)
		CPP.ImGui.SetNextWindowPosCenter(0)
		CPP.ImGui.SetNextWindowFocus();

		CPP.ImGui.BeginFlags("Main", windowFlags)

		if(CPP.ImGui.Button("Proceed"))then
			CPP.interface:PushState("States/mainState.lua")
		end
		CPP.ImGui.SameLine()
		if(CPP.ImGui.Button("Back"))then
			CPP.interface:EntityDelete(main.EID)
		end

		CPP.ImGui.End()

		if(popFont)then
			CPP.ImGui.PopFont()
		end
	end

	return main;
end

return container.NewMain;
