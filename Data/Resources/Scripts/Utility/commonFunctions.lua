local f = {}

function f.AngleToSignedAngle(a)
	if(a>180)then
		a= a - 360;
	end
	return a;
end

function f.SignedAngleToAngle(a)
	if(a<0)then
		a= a + 360;
	end
	return a;
end

function f.InvertHeightMap(hm)
	return 16-hm;
end

function f.TableEmpty(t)
	if next(t) == nil then return true end
	return false
end

function f.PrintTableRecursive(t, recurseLevel)
	local retString = ""
	local recurse = recurseLevel or 0

	if f.TableEmpty(t) then
		retString = retString .. "\n"
		for i=0,recurse do
			--indent
			retString = retString .. "--"
		end
		retString = retString .. "TABLE EMPTY"
		return retString
	end

	for k,v in pairs(t)do
		--newline
		retString = retString .. "\n"
		for i=0,recurse do
			--indent
			retString = retString .. "--"
		end
		retString = retString .. "["..tostring(k).."] = "..tostring(v)
		if(type(v) == "table")then
			retString = retString .. PrintTableRecursive(v, recurse + 1)
		end
	end
	return retString
end

return f
