--Add Moving Platforms to this

--Container name
local collision = {}
collision={};

collision.cComp=nil;

collision.WIDTH=0;
collision.HEIGHT=0;

collision={}
collision.boxID={}
collision.coordinates={}

collision.callbackFunctions={};

	--PrimaryBox
	collision.boxID.BOX_PRIME=0;
	collision.boxPrimary=nil;
	--Entity collision
	collision.boxID.BOX_UP=1;
	collision.boxID.BOX_LEFT=2;
	collision.boxID.BOX_RIGHT=3;
	collision.boxID.BOX_DOWN=4;

	collision.boxEntRight=nil;
	collision.boxEntLeft=nil;
	collision.boxEntUp=nil;
	collision.boxEntDown=nil;

function collision.Init(self, w, h, component, eid)
	self:SetWidthHeight(w,h);
	self.EID = eid
	self.cComp=component

	boxPrimary=	CPP.Rect(0, 0, w+4,	h+4);

	self.boxEntRight=nil;
	self.boxEntLeft=nil;
	self.boxEntUp=nil;
	self.boxEntDown=CPP.Rect(0, (h/2)+2, w,	h/2);

	self.cComp:AddCollisionBox(boxPrimary, self.boxID.BOX_PRIME, 0);
	self.cComp:CheckForEntities(self.boxID.BOX_PRIME);
	self.cComp:SetPrimaryCollisionBox(self.boxID.BOX_PRIME, false);

	self.cComp:AddCollisionBox(self.boxEntDown, self.boxID.BOX_DOWN, 0);
	self.cComp:CheckForEntities(self.boxID.BOX_DOWN);
end

function collision.SetWidthHeight(self, w, h)
	self.WIDTH=w;
	self.HEIGHT=h;
end

return collision;
