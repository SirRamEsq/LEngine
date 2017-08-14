--- ImGui TestBed
-- Use this for a reference as to how to use the CPP.ImGui api

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
		gui.frameCounterMax=500
		gui.frameCounter=0
			
		CPP.interface:ListenForInput(gui.EID, "specialLuaKey");
	end

	function gui.Update()
		local resolution = CPP.interface:GetResolution()
		local windowFlags = 0 -- imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove
		local remap = false;
		CPP.ImGui.PushStyleColorWindowBG(CPP.Color4f(0.2, 0.2, 0.2, 1))
		--Sets ProgressBar BG
		CPP.ImGui.PushStyleColorFrameBG(CPP.Color4f(0, 0.3, 0.3, 1))
		CPP.ImGui.PushStyleColorProgressBarFilled(CPP.Color4f(0, 0.6, 0.6, 1))
		CPP.ImGui.PushStyleColorButton(CPP.Color4f(0, 0, 0.3, 1))
		CPP.ImGui.PushStyleColorButtonActive(CPP.Color4f(0, 0.3, 0, 1))
		CPP.ImGui.PushStyleColorButtonHovered(CPP.Color4f(0, 0.6, 0.6, 1))

		--CPP.ImGui.SetNextWindowSize(gui.defaultWindowSize, 0)
		--CPP.ImGui.SetNextWindowPos(gui.defaultWindowPos, 0)
		CPP.ImGui.BeginFlags("TEST", windowFlags)
			CPP.ImGui.Text("Testing!")
			CPP.ImGui.SameLine()
			CPP.ImGui.Text( "-_-" )
			CPP.ImGui.Text( "Resolution: " .. tostring(resolution.x) .. "x" .. tostring(resolution.y) )

			CPP.ImGui.Sprite(gui.sprite1, "Fire", 0)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 1)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 2)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 3)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 0)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 1)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 2)
			CPP.ImGui.SameLine(); CPP.ImGui.Sprite(gui.sprite1, "Fire", 3)
			CPP.ImGui.Separator()

			CPP.ImGui.Text("More Text")
			local buttonPress = CPP.ImGui.Button("Press This")
			local spriteButtonPress = CPP.ImGui.SpriteButton(gui.sprite1, "Fire", gui.currentFrame)

			CPP.ImGui.Text("Animation ProgressBar")
			CPP.ImGui.ProgressBar(gui.frameCounter / gui.frameCounterMax, CPP.Coord2df(128, 16))
			CPP.ImGui.SameLine()
			CPP.ImGui.Text("-_-")

			local mousePos = CPP.interface:GetMousePosition()
			local mouseWheel = CPP.interface:GetMouseWheel()
			CPP.ImGui.Text("MouseX: " .. tostring(mousePos.x))
			CPP.ImGui.Text("MouseY: " .. tostring(mousePos.y))
			CPP.ImGui.Text("MouseWheel: " .. tostring(mouseWheel))

			if(CPP.interface:GetMouseButtonRight())then
				CPP.ImGui.Text("MouseButtonRight")
			end
			if(CPP.interface:GetMouseButtonMiddle())then
				CPP.ImGui.Text("MouseButtonMiddle")
			end

			remap = CPP.ImGui.Button("Remap Input")

		if(buttonPress == true)then
			CPP.interface:WriteError(gui.EID, "Button Pressed!")
		end
		if(spriteButtonPress == true)then
			CPP.interface:WriteError(gui.EID, "Sprite Button Pressed!")
		end

		gui.frameCounter = gui.frameCounter + 1
		if(gui.frameCounter > gui.frameCounterMax)then
			gui.frameCounter = 0
			gui.currentFrame = gui.currentFrame + 1
		end
		if(gui.currentFrame > gui.maxFrames)then
			gui.currentFrame = 0
		end

		CPP.ImGui.End()
		CPP.ImGui.PopStyleColor(6)

		if(remap == true)then
			local windowFlags2 = imGuiFlags.NoTitleBar + imGuiFlags.NoMove + imGuiFlags.NoResize
			windowFlags2 = windowFlags2 + imGuiFlags.AlwaysAutoResize
			CPP.ImGui.SetNextWindowPosCenter(0)
			CPP.ImGui.SetNextWindowSize(CPP.Coord2df(200,32), 0)
			CPP.ImGui.SetNextWindowFocus();
			CPP.ImGui.BeginFlags("Input", windowFlags2)
				CPP.ImGui.Text("Press Key")
			CPP.ImGui.End()

			CPP.interface:RemapInputToNextKeyPress("specialLuaKey")
		end
	end

	function gui.OnKeyDown(keyname)
		if(keyname=="specialLuaKey") then
			CPP.interface:EntityDelete(gui.EID)
		end
	end

	return gui;
end

return container.NewGui;
