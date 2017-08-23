
local container = {}
function container.NewBoxBreak(baseclass)
	local box={baseclass}
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

	function box.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------
		box.depth		= box.LEngineData.depth;
		box.parent		= box.LEngineData.parent;
		box.EID			= box.LEngineData.entityID;
		box.boxType = box.InitializationTable.boxType or 0
		local EID = box.EID

		box.myPositionComp	= CPP.interface:GetPositionComponent (EID);
		box.myParticleComp	= CPP.interface:GetParticleComponent (EID);

		box.sprite	 = CPP.interface:LoadSprite("SpriteBox.xml");

		if(boxType == 0)then
			box.animation = "box";
		end
		--------------------
		--Particle Effects--
		--------------------
		Vec2d = box.myPositionComp:GetPositionLocal();
		xPos = Vec2d.x;
		yPos = Vec2d.y+10;
		box.particleLifetime = 200;

		box.particleCreator = box.myParticleComp:AddParticleCreator(0, box.particleLifetime);

		local particlePositionMin = CPP.Coord2df(-2, -1);
		local particlePositionMax = CPP.Coord2df( 2,	1);

		local particleVelocityMin = CPP.Coord2df(-0.25,-6.25);
		local particleVelocityMax = CPP.Coord2df( 0.25,-5.75);


		box.particleVelocitySuperMin = CPP.Coord2df(-0.25,-7.25);
		box.particleVelocitySuperMax = CPP.Coord2df( 0.25,-6.75);

		--local particleVelocityMin = CPP.Coord2df(-0.20,-1);
		--local particleVelocityMax = CPP.Coord2df( 0.20, 0);

		local particleAccelMin= CPP.Coord2df(-0.0025, 0.08);
		local particleAccelMax= CPP.Coord2df( 0.0025, 0.08);

		local particleShaderCode= "vec4 luaOut=vec4(fragmentColor.rgb, dotProductUV);\n"

		box.particleCreator:SetPosition(particlePositionMin, particlePositionMax)
		box.particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax)
		box.particleCreator:SetAcceleration(particleAccelMin, particleAccelMax)
		box.particleCreator:SetParticlesPerFrame(10)
		box.particleCreator:SetScalingX(6,8)
		box.particleCreator:SetScalingY(6,8)
		box.particleCreator:SetDepth(box.depth)
		box.particleCreator:SetColor(0.1, 0.6, 0.7, 1.0,	0.2, 0.8, 0.9, 1.0)
		--box.particleCreator:SetColor(0.0, 0.0, 0.0, 1.0,	1.0, 1.0, 1.0, 1.0)
		--box.particleCreator:SetFragmentShaderCode(particleShaderCode)
		box.particleCreator:SetShape(4)
		box.particleCreator:SetEffect(2)

		box.particleCreator:SetWarpQuads(true)

		box.particleCreator:SetSprite(box.sprite)
		box.particleCreator:SetAnimation(box.animation)
		box.particleCreator:SetAnimationSpeed(0)
		box.particleCreator:SetAnimationFrame(0)

		box.particleCreator:Start();
	end
	box.time=0;

	function box.Update()
		box.time = box.time + 1;
		if(box.time > 600)then
			box.particleCreator:SetVelocity(box.particleVelocitySuperMin, box.particleVelocitySuperMax);
			box.time=0;
		end
	end

	return box
end

return container.NewBoxBreak
