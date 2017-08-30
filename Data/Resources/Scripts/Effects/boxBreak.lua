
local container = {}
function container.NewBoxBreak(baseclass)
	--ignore baseclass
	local box = {}
	--Constants
	box.WIDTH=8;
	box.HEIGHT=8;


	--Variables
	box.xPos=0;
	box.yPos=0;

	box.myColComp=nil;
	box.myPositionComp=nil;
	box.myParticleComp=nil;
	box.EID=0;
	box.depth=0;
	box.parent=0;

	box.particleCreator=nil;
	box.particleVelocitySuperMin = nil;
	box.particleVelocitySuperMax = nil;
	box.timer = 4

	function box.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------
		--[[
		box.depth		= box.LEngineData.depth;
		box.parent		= box.LEngineData.parent;
		box.EID			= box.LEngineData.entityID;
		box.boxType = box.LEngineData.InitializationTable.boxType or 0
		local EID = box.EID

		box.mySpriteComp		= CPP.interface:GetSpriteComponent		(EID);
		box.myPositionComp	= CPP.interface:GetPositionComponent (EID);
		box.myParticleComp	= CPP.interface:GetParticleComponent (EID);

		box.sprite	 = CPP.interface:LoadSprite("SpriteBox.xml");

		box.timerMax = 200
		box.timer = 200
		box.animation = "box";

		if(boxType == 0)then
			box.animation = "box";
		end
		box.sprite	 = CPP.interface:LoadSprite("SpriteLouie.xml");
		box.animation = "Stand";
		--]]
		-----------------------
		--C++ Interface setup--
		-----------------------
		box.depth			= box.LEngineData.depth;
		box.parent		= box.LEngineData.parent;
		box.CPPInterface	= CPP.interface
		box.EID			= box.LEngineData.entityID;
		local EID = box.EID

		box.mySpriteComp		= box.CPPInterface:GetSpriteComponent		(EID);
		box.myPositionComp	= box.CPPInterface:GetPositionComponent (EID);
		box.myParticleComp	= box.CPPInterface:GetParticleComponent (EID);


		--box.sprite	 = CPP.interface:LoadSprite("SpriteLouie.xml");
		--box.animation = "Stand";
		box.sprite	 = CPP.interface:LoadSprite("SpriteBox.xml");

		box.timerMax = 200
		box.timer = 200
		box.animation = "box";
		--------------------
		--Particle Effects--
		--------------------
		Vec2d = box.myPositionComp:GetPositionLocal();
		xPos = Vec2d.x;
		yPos = Vec2d.y+10;
		box.particleLifetime = 40;

		box.particleCreator = box.myParticleComp:AddParticleCreator(5, box.particleLifetime);

		local particlePositionMin = CPP.Coord2df(-2, -1);
		local particlePositionMax = CPP.Coord2df( 2,	1);

		local particleVelocityMin = CPP.Coord2df(-2.25, -0.25);
		local particleVelocityMax = CPP.Coord2df(2.75,  0.25);

		local particleAccelMin= CPP.Coord2df(-0.0025, 0.01);
		local particleAccelMax= CPP.Coord2df( 0.0025, 0.01);

		local particleShaderCode= "vec4 luaOut=vec4(fragmentColor.rgb, dotProductUV);\n"

		box.particleCreator:SetPosition(particlePositionMin, particlePositionMax);
		box.particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax);
		box.particleCreator:SetAcceleration(particleAccelMin, particleAccelMax);
		box.particleCreator:SetParticlesPerFrame(10);
		box.particleCreator:SetScalingX(2,1);
		box.particleCreator:SetScalingY(2,1);
		box.particleCreator:SetDepth(box.depth);
		--box.particleCreator:SetColor(0.1, 0.6, 0.7, 0.1,	0.2, 0.8, 0.9, 1.0);
		box.particleCreator:SetColor(0.75, 0.75, 0.75, 0.75,	1, 1, 1, 1);
		--box.particleCreator:SetColor(0.0, 0.0, 0.0, 1.0,	1.0, 1.0, 1.0, 1.0);
		--box.particleCreator:SetFragmentShaderCode(particleShaderCode);
		
		box.particleCreator:SetSprite(box.sprite)
		box.particleCreator:SetAnimation(box.animation)
		box.particleCreator:SetAnimationFrame(0)
		box.particleCreator:SetRandomUV(true)
		box.particleCreator:SetWarpQuads(true)
		
		box.particleCreator:SetShape(4);
		box.particleCreator:SetEffect(2);
		box.particleCreator:Start();
	end

	function box.Update()
		box.timer = box.timer - 1;
		if(box.timer < 0)then
			--CPP.interface:EntityDelete(box.EID)
		end
	end

	return box
end

return container.NewBoxBreak