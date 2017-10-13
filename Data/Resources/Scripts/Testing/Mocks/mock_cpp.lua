function mockCPP()
	local mock = {}

	function mock.WriteError(self, eid, errorString)
		error("[EID " .. eid .. "] " .. errorString)
	end

	function mock.Rect(x, y, w, h)
		local rect = {}
		rect.x=x
		rect.y=y
		rect.w=w
		rect.h=h

		function rect.GetTop(self)
			if (h>=0) then return self.y end
			return self.y + self.h
		end
		function rect.GetBottom(self)
			if (h<=0) then return self.y end
			return self.y + self.h
		end
		function rect.GetLeft(self)
			if (w>=0) then return self.x end
			return self.x + self.w
		end
		function rect.GetRight(self)
			if (w<=0) then return self.x end
			return self.x + self.w
		end
		return rect
	end

	function mock.Coord2df(x,y)
		local coord = {}
		coord.x=x
		coord.y=y

		return coord
	end

	return mock
end
return mockCPP()
