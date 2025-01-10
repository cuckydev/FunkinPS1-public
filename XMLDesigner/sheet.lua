-- Modules
local nativefs = require("nativefs")
local findbyname = require("findbyname")

-- Sheet processor class
local sheet = {}
sheet.__index = sheet

function sheet.new(xml, name)
	local self = setmetatable({}, sheet)
	
	-- Search for TextureAtlas tag
	self.xml = xml
	self.xml_atlas = findbyname(self.xml, "TextureAtlas")
	if self.xml_atlas == nil then
		return nil
	end
	
	-- Read image file
	local file_path, _, _ = name:match("(.-)([^/]-([^/%.]+))$")
	local image_file, image_file_err = nativefs.newFileData(file_path..findbyname(self.xml_atlas.attr, "imagePath").value)
	if image_file == nil then
		return nil
	end
	
	local image_data = love.image.newImageData(image_file)
	self.xml_image = love.graphics.newImage(image_data)
	
	-- Process frames
	self.sheet_frames = {}
	for _,v in pairs(self.xml_atlas.el) do
		if v.name == "SubTexture" then
			local sheet_frame = {}

			sheet_frame.name = findbyname(v.attr, "name").value
			sheet_frame.x = tonumber(findbyname(v.attr, "x").value)
			sheet_frame.y = tonumber(findbyname(v.attr, "y").value)
			sheet_frame.width = tonumber(findbyname(v.attr, "width").value)
			sheet_frame.height = tonumber(findbyname(v.attr, "height").value)
			sheet_frame.frameX = tonumber(findbyname(v.attr, "frameX", {value = 0}).value)
			sheet_frame.frameY = tonumber(findbyname(v.attr, "frameY", {value = 0}).value)
			sheet_frame.frameWidth = tonumber(findbyname(v.attr, "frameWidth", {value = sheet_frame.width}).value)
			sheet_frame.frameHeight = tonumber(findbyname(v.attr, "frameHeight", {value = sheet_frame.height}).value)

			table.insert(self.sheet_frames, sheet_frame)
		end
	end

	self.view_i = 1
	
	return self
end

function sheet:keypressed(key, scancode, isrepeat)
	if key == "left" then
		if self.sheet_frames[self.view_i - 1] ~= nil then
			self.view_i = self.view_i - 1
		end
	elseif key == "right" then
		if self.sheet_frames[self.view_i + 1] ~= nil then
			self.view_i = self.view_i + 1
		end
	end
end

function sheet:mousepressed(x, y, button, istouch, presses)
	
end

function sheet:mousereleased(x, y, button, istouch, presses)
	
end

function sheet:mousemoved(x, y, dx, dy, istouch)
	
end

function sheet:draw()
	local sheet_frame = self.sheet_frames[self.view_i]
	love.graphics.print(sheet_frame.name, 4, -32)
	
	local subtex_x = sheet_frame.x
	local subtex_y = sheet_frame.y
	local subtex_w = sheet_frame.width
	local subtex_h = sheet_frame.height

	local subtex_fx = sheet_frame.frameX
	local subtex_fy = sheet_frame.frameY
	local subtex_fw = sheet_frame.frameWidth
	local subtex_fh = sheet_frame.frameHeight

	if subtex_fx == nil then
		subtex_fx = 0
	end
	if subtex_fy == nil then
		subtex_fy = 0
	end
	if subtex_fw == nil then
		subtex_fw = subtex_w
	end
	if subtex_fh == nil then
		subtex_fh = subtex_h
	end

	local subtex_quad = love.graphics.newQuad(subtex_x, subtex_y, subtex_w, subtex_h, self.xml_image)
	love.graphics.draw(self.xml_image, subtex_quad, -subtex_fx, -subtex_fy)
	
	-- Get coordinates
	local mx = love.mouse.getX()
	local my = love.mouse.getY()
	mx, my = love.graphics.inverseTransformPoint(mx, my)
	
	love.graphics.print(string.format("%d,%d", mx, my), 4, -48)
end

return sheet
