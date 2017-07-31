--- Main Menu GUI
-- Description goes here



local container = {}
function container.NewGui(baseclass)
	local gui = baseclass or {}

	--Ignore baseclass
	--gui = {}

	function gui.Initialize()
		gui.depth		= gui.LEngineData.depth;
		gui.parent		= gui.LEngineData.parent;
		gui.CPPInterface = CPP.interface
		gui.EID			= gui.LEngineData.entityID;

		CPP.interface:WriteError(gui.EID, "BeganInit");
		
		local defaultWindowSizeW = 100
		local defaultWindowSizeH = 100

		local defaultWindowPosX = 20
		local defaultWindowPosY = 20

		local defaultButtonSizeW = 64
		local defaultButtonSizeH = 32

		gui.defaultWindowSize =	CPP.Coord2df(defaultWindowSizeW, defaultWindowSizeH)
		gui.defaultWindowPos =	CPP.Coord2df(defaultWindowPosX, defaultWindowPosY)
		gui.defaultButtonSize = CPP.Coord2df(defaultButtonSizeW, defaultButtonSizeH)
	end

	function gui.Update()
	--[[	CPP.interface:WriteError(11, "TEST2");
		CPP.ImGui.SetNextWindowSize(gui.defaultWindowSize, 0)
		CPP.ImGui.SetNextWindowPos(gui.defaultWindowPos, 0)

		CPP.ImGui.Begin("TEST")

		CPP.ImGui.Text("Testing!")
		local buttonPress = CPP.ImGui.Button("Press This", gui.defaultButtonSize)
		CPP.ImGui.Text("More Text")

		CPP.ImGui.End()

		if(buttonPress == true)then
			CPP.WriteError(gui.EID, "Button Pressed!")
		end--]]
	end

	return gui;
end

return container.NewGui;
