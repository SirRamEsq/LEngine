local container = {}
function container.NewFallingHat(baseclass)
	--ignore baseclass
	local hat = {}
	--Constants
	hat.WIDTH=8;
	hat.HEIGHT=8;


	--Variables
	hat.xPos=0;
	hat.yPos=0;

	hat.myColComp=nil;
	hat.CompPosition=nil;
	hat.CompParticle=nil;
	hat.EID=0;
	hat.depth=0;
	hat.parent=0;

	hat.particleCreator=nil;
	hat.particleVelocitySuperMin = nil;
	hat.particleVelocitySuperMax = nil;
	hat.timer = 4

	hat.c = {}
	hat.c.GRAVITY=0.21875;
	hat.c.VELOCITY_Y = -6
	hat.c.VELOCITY_X = 1

	function hat.Initialize()
		-----------------------
		--C++ Interface setup--
		-----------------------
		hat.depth		= hat.LEngineData.depth;
		hat.parent		= hat.LEngineData.parent;
		hat.EID			= hat.LEngineData.entityID;
		local EID = hat.EID

		hat.CompSprite		= CPP.interface:GetSpriteComponent   (EID);
		hat.CompPosition	= CPP.interface:GetPositionComponent (EID);
		hat.CompParticle	= CPP.interface:GetParticleComponent (EID);
		hat.dir = hat.LEngineData.InitializationTable.direction or 1

		hat.sprite	 = CPP.interface:LoadSprite("louieHat.xml");
		hat.spriteID = hat.CompSprite:AddSprite(hat.sprite, hat.depth, hat.sprite:Width()/2 ,hat.sprite:Height()/2);
		hat.animation = "hat"
		hat.rotation = 0
		hat.rotationSpeed = 2.5

		hat.CompSprite:SetAnimation		(hat.spriteID, hat.animation);
		hat.CompSprite:SetAnimationSpeed(hat.spriteID, 0);
		hat.CompSprite:SetRotation		(hat.spriteID, 0);

		hat.deleteTimer = 600

		--------------------
		--Particle Effects--
		--------------------
		hat.particleLifetime = 90;

		hat.particleCreator = hat.CompParticle:AddParticleCreator(0, hat.particleLifetime);

		local particlePositionMin = CPP.Coord2df(-8, -8);
		local particlePositionMax = CPP.Coord2df( 8,  8);

		local particleVelocityMin = CPP.Coord2df(-0.25, -0.25);
		local particleVelocityMax = CPP.Coord2df(0.25,  -1.00);

		local particleAccelMin= CPP.Coord2df(-0.0025, 0.01);
		local particleAccelMax= CPP.Coord2df( 0.0025, 0.01);

		hat.particleCreator:SetPosition(particlePositionMin, particlePositionMax);
		hat.particleCreator:SetVelocity(particleVelocityMin, particleVelocityMax);
		hat.particleCreator:SetAcceleration(particleAccelMin, particleAccelMax);
		hat.particleCreator:SetParticlesPerFrame(.25);
		hat.particleCreator:SetScalingX(2,2);
		hat.particleCreator:SetScalingY(2,2);
		hat.particleCreator:SetDepth(hat.depth);
		hat.particleCreator:SetColor(0.9, 0.2, 0.2, 1,	1, .1, .1, 1);
		
		hat.particleCreator:SetShape(4);
		hat.particleCreator:SetEffect(2);
		hat.particleCreator:Start();
		hat.CompPosition:SetMovement(CPP.Coord2df(hat.c.VELOCITY_X * hat.dir, hat.c.VELOCITY_Y));
		hat.CompPosition:SetAcceleration(CPP.Coord2df(0,hat.c.GRAVITY));
	end

	function hat.Update()
		hat.CompSprite:SetRotation(hat.spriteID, hat.rotation);
		hat.rotation = hat.rotation + hat.rotationSpeed
		hat.rotation = hat.rotation % 360

		hat.deleteTimer = hat.deleteTimer - 1
		if(hat.deleteTimer <=0)then
			CPP.interface:EntityDelete(hat.EID)
		end
	end

	return hat
end

return container.NewFallingHat
