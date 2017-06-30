function mockCPP()
	local mock = {}

	function mock.CRect(x, y, w, h)
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

	return mock
end
return mockCPP()
