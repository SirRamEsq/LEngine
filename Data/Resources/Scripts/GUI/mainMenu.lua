--- Main Menu GUI
-- Description goes here


local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewGui(baseclass)
	local gui = baseclass or {}

	--Ignore baseclass
	gui = {}

	function gui.Initialize()
		gui.depth		= gui.LEngineData.depth;
		gui.parent		= gui.LEngineData.parent;
		gui.CPPInterface = CPP.interface
		gui.EID			= gui.LEngineData.entityID;

		local defaultWindowSizeW = 200
		local defaultWindowSizeH = 200

		local defaultWindowPosX = 20
		local defaultWindowPosY = 20

		local defaultButtonSizeW = 128
		local defaultButtonSizeH = 32

		gui.defaultWindowSize =	CPP.Coord2df(defaultWindowSizeW, defaultWindowSizeH)
		gui.defaultWindowPos =	CPP.Coord2df(defaultWindowPosX, defaultWindowPosY)
		gui.defaultButtonSize = CPP.Coord2df(defaultButtonSizeW, defaultButtonSizeH)
	
		gui.sprite1 = CPP.interface:LoadSprite("SpriteArrow.xml");
		if(gui.sprite1 == nil)then
			CPP.interface:WriteError(gui.EID, "Sprite is NIL!")
		end
		gui.maxFrames=3
		gui.currentFrame=0
	end

	function gui.Update()
		local windowFlags = 0--imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove
		--CPP.ImGui.SetNextWindowSize(gui.defaultWindowSize, 0)
		--CPP.ImGui.SetNextWindowPos(gui.defaultWindowPos, 0)
		CPP.ImGui.BeginFlags("TEST", windowFlags)
			CPP.ImGui.Text("Testing!")
			CPP.ImGui.Text("More Text")
			local buttonPress = CPP.ImGui.Button("Press This")
		CPP.ImGui.Sprite(gui.sprite1, "Fire", gui.currentFrame)
		CPP.ImGui.End()

		if(buttonPress == true)then
			CPP.interface:WriteError(gui.EID, "Button Pressed!")
		end
		gui.currentFrame = gui.currentFrame + 1
		if(gui.currentFrame > gui.maxFrames)then
			gui.currentFrame = 0
		end
	end

	return gui;
end

return container.NewGui;
