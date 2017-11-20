local commonFunctions = {}

function commonFunctions.AngleToSignedAngle(a)
	if(a>180)then
		a= a - 360;
	end
	return a;
end

function commonFunctions.SignedAngleToAngle(a)
	if(a<0)then
		a= a + 360;
	end
	return a;
end

function commonFunctions.InvertHeightMap(hm)
	return 16-hm;
end

return commonFunctions
