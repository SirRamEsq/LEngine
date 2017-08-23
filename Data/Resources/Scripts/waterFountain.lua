function NewFountain(baseclass)
	local water={baseclass}
	--Constants
	water.WIDTH=8;
	water.HEIGHT=8;


	--Variables
	water.xPos=0;
	water.yPos=0;
	--C++ Interfacing
	water.CPPInterface=nil;
	water.mySprite=nil;
	water.mySpriteComp=nil;
	water.myColComp=nil;
	water.myPositionComp=nil;
	water.myParticleComp=nil;
	water.EID=0;
	water.depth=0;
	water.parent=0;

	water.particleCreator=nil;
	water.particleVelocitySuperMin = nil;
	water.particleVelocitySuperMax = nil;

	function water.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------
		water.depth			= water.LEngineData.depth;
		water.parent		= water.LEngineData.parent;
		water.CPPInterface	= CPP.interface
		water.EID			= water.LEngineData.entityID;
		local EID = water.EID

		water.mySpriteComp		= water.CPPInterface:GetSpriteComponent		(EID);
		water.myPositionComp	= water.CPPInterface:GetPositionComponent (EID);
		water.myParticleComp	= water.CPPInterface:GetParticleComponent (EID);


		water.sprite	 = CPP.interface:LoadSprite("SpriteLouie.xml");
		water.animation = "Stand";
		--------------------
		--Particle Effects--
		--------------------
		Vec2d = water.myPositionComp:GetPositionLocal();
		xPos = Vec2d.x;
		yPos = Vec2d.y+10;
		water.particleLifetime = 200;

		water.particleCreator = water.myParticleComp:AddParticleCreator(0, water.particleLifetime);

		local particlePositionMin = CPP.Coord2df(-2, -1);
		local particlePositionMax = CPP.Coord2df( 2,	1);

		local particleVelocityMin = CPP.Coord2df(-0.25,-6.25);
		local particleVelocityMax = CPP.Coord2df( 0.25,-5.75);


		water.particleVelocitySuperMin = CPP.Coord2df(-0.25,-7.25);
		water.particleVelocitySuperMax = CPP.Coord2df( 0.25,-6.75);

		--local particleVelocityMin = CPP.Coord2df(-0.20,-1);
		--local particleVelocityMax = CPP.Coord2df( 0.20, 0);

		local particleAccelMin= CPP.Coord2df(-0.0025, 0.08);
		local particleAccelMax= CPP.Coord2df( 0.0025, 0.08);

		local particleShaderCode= "vec4 luaOut=vec4(fragmentColor.rgb, dotProductUV);\n"

		water.particleCreator:SetPosition(particlePositionMin, particlePositionMax);
		water.particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax);
		water.particleCreator:SetAcceleration(particleAccelMin, particleAccelMax);
		water.particleCreator:SetParticlesPerFrame(10);
		water.particleCreator:SetScalingX(6,8);
		water.particleCreator:SetScalingY(6,8);
		water.particleCreator:SetDepth(water.depth);
		water.particleCreator:SetColor(0.1, 0.6, 0.7, 1.0,	0.2, 0.8, 0.9, 1.0);
		--water.particleCreator:SetColor(0.0, 0.0, 0.0, 1.0,	1.0, 1.0, 1.0, 1.0);
		--water.particleCreator:SetFragmentShaderCode(particleShaderCode);
		
		water.particleCreator:SetSprite(water.sprite)
		water.particleCreator:SetAnimation(water.animation)
		water.particleCreator:SetAnimationSpeed(0)
		water.particleCreator:SetAnimationFrame(0)
		
		water.particleCreator:SetShape(4);
		water.particleCreator:SetEffect(2);
		water.particleCreator:Start();
	end
	water.time=0;

	function water.Update()
		water.time = water.time + 1;
		if(water.time > 600)then
			water.particleCreator:SetVelocity(water.particleVelocitySuperMin, water.particleVelocitySuperMax);
			water.time=0;
		end
	end

	return water
end

return NewFountain
