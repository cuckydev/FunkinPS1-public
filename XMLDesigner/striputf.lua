return function(data)
	-- Strip UTF code (bad)
	local data_src = data
	data = ""
	for i = 1, data_src:len() do
		local c = data_src:sub(i, i)
		if c:byte() < 0x80 then
			data = data..c
		end
	end
	return data
end
