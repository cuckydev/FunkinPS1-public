return function(tag, name, default)
	for _,v in pairs(tag) do
		if v.name == name then
			return v
		end
	end
	return default
end
