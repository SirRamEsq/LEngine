-- ImGui TestBed
-- Use this for a reference as to how to use the CPP.ImGui api

local result=0;
result, imGuiFlags = pcall(loadfile(commonPath .. "/imGuiWindowFlags.lua", _ENV))

local container = {}
function container.NewGui(baseclass)
	local gui = baseclass or {}

	--Ignore baseclass
	gui = {}

	function gui.LoadFont()
		local popFont = CPP.ImGui.PushFont(gui.font, gui.fontSize)

		if(popFont)then
			CPP.ImGui.PopFont(1)
		end
	end

	function gui.Initialize()
		gui.depth		= gui.LEngineData.depth;
		gui.parent		= gui.LEngineData.parent;
		gui.CPPInterface = CPP.interface
		gui.EID			= gui.LEngineData.entityID;
		local EID = gui.EID

		local defaultWindowSizeW = 200
		local defaultWindowSizeH = 200

		local defaultWindowPosX = 20
		local defaultWindowPosY = 20

		local defaultButtonSizeW = 128
		local defaultButtonSizeH = 32

		gui.defaultWindowSize =	CPP.Coord2df(defaultWindowSizeW, defaultWindowSizeH)
		gui.defaultWindowPos =	CPP.Coord2df(defaultWindowPosX, defaultWindowPosY)
		gui.defaultButtonSize = CPP.Coord2df(defaultButtonSizeW, defaultButtonSizeH)
		gui.winSize = CPP.Coord2df(0,0)

		gui.winName = "TEST1"
	
		gui.sprite1 = CPP.interface:LoadSprite("SpriteArrow.xml");
		if(gui.sprite1 == nil)then
			CPP.interface:WriteError(gui.EID, "Sprite is NIL!")
		end
		gui.maxFrames=3
		gui.currentFrame=0
		gui.frameCounterMax=200
		gui.frameCounter=0
		gui.noBG=false
		gui.translateY = 200
		gui.simulateKeyPress = true

		gui.font = "ebFonts/wisdom.ttf"
		gui.fontSize = 30
		--load font before remap key button is pushed
		gui.LoadFont()
			
		CPP.interface:ListenForInput(gui.EID, "specialLuaKey");
		gui.myPositionComp	= CPP.interface:GetPositionComponent (EID);
		gui.myParticleComp	= CPP.interface:GetParticleComponent (EID);
		gui.myCameraComp	= CPP.interface:GetCameraComponent (EID);

		--just for giggles
		local resolution = CPP.interface:GetResolution()
		--gui.myCameraComp:SetViewport(CPP.CRect(0,0,resolution.x,resolution.y))

		--------------------
		--Particle Effects--
		--------------------
		gui.sprite	 = CPP.interface:LoadSprite("SpriteArrow.xml");
		gui.animation = "Fire";
		Vec2d = gui.myPositionComp:GetPositionLocal();
		local xPos = Vec2d.x+200;
		local yPos = Vec2d.y+250;
		gui.particleLifetime = 150;

		gui.particleCreator = gui.myParticleComp:AddParticleCreator(0, gui.particleLifetime);

		local particlePositionMin = CPP.Coord2df(xPos-8, yPos-8);
		local particlePositionMax = CPP.Coord2df(xPos+8, yPos+8);

		--local particleVelocityMin = CPP.Coord2df(-0.25, -0.25);
		--local particleVelocityMax = CPP.Coord2df(0.75,  0.25);
		local particleVelocityMin = CPP.Coord2df(-0.01, -0.01);
		local particleVelocityMax = CPP.Coord2df( 0.01,  0.01);

		local particleAccelMin= CPP.Coord2df(-0.0025, 0.01);
		local particleAccelMax= CPP.Coord2df( 0.0025, 0.01);

		gui.particleCreator:SetPosition(particlePositionMin, particlePositionMax);
		gui.particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax);
		gui.particleCreator:SetAcceleration(particleAccelMin, particleAccelMax);
		gui.particleCreator:SetParticlesPerFrame(1);
		gui.particleCreator:SetScalingX(8,8);
		gui.particleCreator:SetScalingY(8,8);
		gui.particleCreator:SetDepth(gui.depth);
		gui.particleCreator:SetColor(0.1, 0.6, 0.7, 0.1,	0.2, 0.8, 0.9, 1.0);
		gui.particleCreator:SetColor(1, 1, 1, 1,	1, 1, 1, 1);
		--gui.particleCreator:SetColor(0.0, 0.0, 0.0, 1.0,	1.0, 1.0, 1.0, 1.0);
		
		--gui.particleCreator:SetSprite(gui.sprite)
		--gui.particleCreator:SetAnimation(gui.animation)
		--gui.particleCreator:SetAnimationFrame(0)
		--gui.particleCreator:SetUsePoint(true)
		--gui.particleCreator:SetPoint(CPP.Coord2df(xPos,yPos+8))
		--gui.particleCreator:SetPointIntensity(50)
		gui.particleCreator:SetRandomUV(false)
		gui.particleCreator:SetWarpQuads(false)
		
		gui.particleCreator:SetShape(4);
		gui.particleCreator:SetEffect(2);
		gui.particleCreator:Start();

		gui.particleTimerMax = 500
		gui.particleTimer = gui.particleTimerMax
	end

	function gui.Update()
		local resolution = CPP.interface:GetResolution()
		local windowFlags = imGuiFlags.NoTitleBar + imGuiFlags.NoResize + imGuiFlags.NoMove + imGuiFlags.AlwaysAutoResize
		local remap = false;
		if(gui.noBG == false)then
			CPP.ImGui.PushStyleColorWindowBG(CPP.Color4f(0.2, 0.2, 0.2, 1))
		else
			CPP.ImGui.PushStyleColorWindowBG(CPP.Color4f(0,0,0, 0))
		end
		--Sets ProgressBar BG
		CPP.ImGui.PushStyleColorFrameBG(CPP.Color4f(0, 0.3, 0.3, 1))
		local progressBarFill = gui.frameCounter /gui.frameCounterMax
		CPP.ImGui.PushStyleColorProgressBarFilled(CPP.Color4f(1 - progressBarFill, progressBarFill, 0, 1))
		CPP.ImGui.PushStyleColorButton(CPP.Color4f(0, 0, 0.3, 1))
		CPP.ImGui.PushStyleColorButtonActive(CPP.Color4f(0, 0.3, 0, 1))
		CPP.ImGui.PushStyleColorButtonHovered(CPP.Color4f(0, 0.6, 0.6, 1))

		--CPP.ImGui.SetNextWindowSize(gui.defaultWindowSize, 0)
		--CPP.ImGui.SetNextWindowPos(gui.defaultWindowPos, 0)
		CPP.ImGui.BeginFlags(gui.winName, windowFlags)
			CPP.ImGui.Text("Testing!")
			CPP.ImGui.SameLine()
			CPP.ImGui.Text( "-_-" )
			CPP.ImGui.Text( "Resolution: " .. tostring(resolution.x) .. "x" .. tostring(resolution.y) )
			CPP.ImGui.Text( "This Window Size: " .. tostring(gui.winSize.x) .. "x" .. tostring(gui.winSize.y) )

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
			CPP.ImGui.ProgressBar(progressBarFill, CPP.Coord2df(128, 16))
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


		gui.winSize = CPP.ImGui.GetWindowSize()
		CPP.ImGui.End()
		CPP.ImGui.PopStyleColor(6)


		--Center Window
		gui.currentPosition = CPP.Coord2df(( resolution.x/2) - (gui.winSize.x/2), gui.translateY)

		--Left Align Window
		if(CPP.interface:GetMouseButtonMiddle())then
			gui.currentPosition = CPP.Coord2df(0, gui.translateY)
			if(gui.simulateKeyPress == false) then
				CPP.interface:SimulateKeyPress("specialLuaKey")
				CPP.interface:SimulateKeyRelease("specialLuaKey")
				
				gui.simulateKeyPress = true
			end
		else
			gui.simulateKeyPress = false
		end

		--Right Align Window
		if(CPP.interface:GetMouseButtonRight())then
			 gui.currentPosition = CPP.Coord2df(( resolution.x) - (gui.winSize.x), gui.translateY)
		end

		gui.translateY = gui.translateY - (CPP.interface:GetMouseWheel()*5)
		CPP.ImGui.SetWindowPos(gui.winName, gui.currentPosition, 0)

		if(remap == true)then
			local windowFlags2 = imGuiFlags.NoTitleBar + imGuiFlags.NoMove + imGuiFlags.NoResize + imGuiFlags.AlwaysAutoResize
			windowFlags2 = windowFlags2 + imGuiFlags.AlwaysAutoResize
			local popFont = CPP.ImGui.PushFont(gui.font, gui.fontSize)
			local newPos = CPP.Coord2df(0,0) 

			CPP.ImGui.SetNextWindowPos(newPos, 0)
			CPP.ImGui.SetNextWindowSize(resolution, 0)
			CPP.ImGui.SetNextWindowFocus();
			CPP.ImGui.PushStyleColorWindowBG(CPP.Color4f(0, 0.3, 0.3, 1))
			CPP.ImGui.BeginFlags("Input", windowFlags2)
				CPP.ImGui.Text("Press Key")
			CPP.ImGui.End()

			CPP.interface:RemapInputToNextKeyPress("specialLuaKey")
			CPP.ImGui.PopStyleColor(1)
			if(popFont)then
				CPP.ImGui.PopFont(1)
			end
		end

		gui.particleTimer = gui.particleTimer - 1
		if(gui.particleTimer <= 0 )then
			gui.particleTimer = gui.particleTimerMax;
			--gui.particleCreator:Start()
		end
	end

	function gui.OnKeyDown(keyname)
		if(keyname=="specialLuaKey") then
			gui.noBG = not gui.noBG
		end
	end

	return gui;
end

return container.NewGui;
