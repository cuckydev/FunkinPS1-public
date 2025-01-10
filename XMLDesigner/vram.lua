-- Modules
local slaxdom = require("slaxdom")
local findbyname = require("findbyname")
local nativefs = require("nativefs")
local striputf = require("striputf")
local hsl = require("hsl")

-- VRAM processor class
local vram = {}
vram.__index = vram

function vram.new(xml, name)
	local self = setmetatable({}, vram)
	
	-- Search for chr tag
	self.xml = xml
	self.name = name

	self.xml_chr = findbyname(self.xml.kids, "chr")
	if self.xml_chr == nil then
		self.xml_chr = findbyname(self.xml.kids, "spr")
		if self.xml_chr == nil then
			return nil
		end
	end

	-- Load grid.png image
	self.grid_image = love.graphics.newImage("grid.png", { mipmaps = true })
	self.grid_image:setWrap("repeat", "repeat")
	self.grid_image:setFilter("linear", "nearest")

	self.grid_quad = love.graphics.newQuad(0, 0, 1024, 512, 1, 1)
	
	-- Read chr attributes
	self.scale = tonumber(findbyname(self.xml_chr.attr, "scale").value)
	self.highbpp = tonumber(findbyname(self.xml_chr.attr, "highbpp").value) == 1
	
	-- Read sheet xml
	local file_path, _, _ = name:match("(.-)([^/]-([^/%.]+))$")
	local file = nativefs.newFile(file_path..(findbyname(self.xml_chr.attr, "sheet").value))
	file:open("r")
	local xml_data = striputf(file:read())
	file:close()

	-- Parse sheet xml
	local sheet_xml = slaxdom:dom(xml_data)
	
	-- Process sheet frames
	self.sheet_frames = {}
	
	local texture_atlas = findbyname(sheet_xml.kids, "TextureAtlas")
	for _,v in ipairs(texture_atlas.el) do
		if v.name == "SubTexture" then
			local sheet_frame = {}

			sheet_frame.x = tonumber(findbyname(v.attr, "x").value)
			sheet_frame.y = tonumber(findbyname(v.attr, "y").value)
			sheet_frame.width = tonumber(findbyname(v.attr, "width").value)
			sheet_frame.height = tonumber(findbyname(v.attr, "height").value)
			sheet_frame.frameX = tonumber(findbyname(v.attr, "frameX", {value = 0}).value)
			sheet_frame.frameY = tonumber(findbyname(v.attr, "frameY", {value = 0}).value)
			sheet_frame.frameWidth = tonumber(findbyname(v.attr, "frameWidth", {value = sheet_frame.width}).value)
			sheet_frame.frameHeight = tonumber(findbyname(v.attr, "frameHeight", {value = sheet_frame.height}).value)

			self.sheet_frames[findbyname(v.attr, "name").value] = sheet_frame
		end
	end

	-- Read image file
	local sheet_path, _, _ = (file_path..(findbyname(self.xml_chr.attr, "sheet").value)):match("(.-)([^/]-([^/%.]+))$")
	local image_file, _ = nativefs.newFileData(sheet_path..(texture_atlas.attr.imagePath))
	if image_file == nil then
		return nil
	end
	
	local image_data = love.image.newImageData(image_file)
	self.xml_image = love.graphics.newImage(image_data)
	
	-- Process frames
	self.frames = {}
	
	for _,v in pairs(self.xml_chr.el) do
		if v.name == "frame" then
			local frame = {}
			frame.original = v

			frame.source = findbyname(v.attr, "source").value

			local source = self.sheet_frames[frame.source]
			frame.sw = source.width
			frame.sh = source.height

			frame.flip = tonumber(findbyname(v.attr, "flip", {value = 0}).value)

			frame.ax = tonumber(findbyname(v.attr, "ax", {value = 0}).value) + source.frameX
			frame.ay = tonumber(findbyname(v.attr, "ay", {value = 0}).value) + source.frameY

			frame.tx = tonumber(findbyname(v.attr, "tx", {value = 0}).value)
			frame.ty = tonumber(findbyname(v.attr, "ty", {value = 0}).value)
			frame.cx = tonumber(findbyname(v.attr, "cx", {value = 0}).value)
			frame.cy = tonumber(findbyname(v.attr, "cy", {value = 0}).value)

			frame.w = math.ceil(frame.sw * self.scale)
			frame.h = math.ceil(frame.sh * self.scale)
			
			if self.highbpp then
				frame.w = math.floor((frame.w + 1) / 2)
			else
				frame.w = math.floor((frame.w + 3) / 4)
			end

			frame.quad = love.graphics.newQuad(source.x, source.y, source.width, source.height, self.xml_image:getDimensions())

			table.insert(self.frames, frame)
		end
	end
	
	-- Grip state
	self.select_frame = nil

	self.grip_frame = nil
	self.grip_x = 0
	self.grip_y = 0
	
	return self
end

function vram:keypressed(key, scancode, isrepeat)
	-- Check if ctrl+s is pressed
	if key == "s" and love.keyboard.isDown("lctrl") then
		-- Confirm save using love.window.showMessageBox
		local button = love.window.showMessageBox("Save", "Save changes?", {"No", "Yes", enterbutton = 2, escapebutton = 1})
		if button == 2 then
			-- Apply changes to dom
			for _,v in pairs(self.frames) do
				findbyname(v.original.attr, "tx").value = tostring(v.tx)
				findbyname(v.original.attr, "ty").value = tostring(v.ty)
				findbyname(v.original.attr, "cx").value = tostring(v.cx)
				findbyname(v.original.attr, "cy").value = tostring(v.cy)
			end

			-- Save dom
			local file = nativefs.newFile(self.name)
			file:open("w")
			file:write(slaxdom:xml(self.xml))
			file:write("\n")
			file:close()
		end
	end
end

function vram:mousepressed(x, y, button, istouch, presses)
	if button == 1 then
		-- Get mouse position
		x, y = love.graphics.inverseTransformPoint(x, y)

		-- Find frame
		self.select_frame = nil
		self.grip_frame = nil

		for _,v in pairs(self.frames) do
			if x >= tonumber(v.tx) and y >= tonumber(v.ty) and x < tonumber(v.tx) + v.w and y < tonumber(v.ty) + v.h then
				-- Select frame
				self.select_frame = v

				-- Grip frame
				self.grip_frame = v
				self.grip_x = v.tx - x
				self.grip_y = v.ty - y
				return
			end
		end
	end
end

function vram:mousereleased(x, y, button, istouch, presses)
	if button == 1 then
		-- Release frame
		self.grip_frame = nil
	end
end

function vram:mousemoved(x, y, dx, dy, istouch)
	if self.grip_frame ~= nil then
		-- Get mouse position
		x, y = love.graphics.inverseTransformPoint(x, y)

		-- Move frame
		self.grip_frame.tx = math.floor(x + self.grip_x + 0.5)
		self.grip_frame.ty = math.floor(y + self.grip_y + 0.5)
	end
end

function vram:draw()
	-- Draw vram pages
	for y = 0, 1 do
		for x = 0, 15 do
			local b = 0.2 + ((y + x) % 2) * 0.1
			love.graphics.setColor(b, b, b, 0.9)
			love.graphics.rectangle("fill", x * 64, y * 256, 64, 256)
		end
	end
	
	-- Draw frames
	for i,v in pairs(self.frames) do
		-- Calculate frame unique color
		local sel_br = (v == self.select_frame) and 1 or 0.75
		local r, g, b = hsl((i * 2.713455) % 1, sel_br, sel_br * 0.5)
		
		-- Draw CLUT frame
		love.graphics.setColor(r, g, b, 1)
		love.graphics.rectangle("fill", v.cx, v.cy, self.highbpp and 256 or 16, 1)

		-- Draw image frame
		love.graphics.setColor(r, g, b, 0.5)
		love.graphics.rectangle("fill", v.tx, v.ty, v.w, v.h)

		-- Draw image
		local tx = v.tx
		local ty = v.ty
		local sx = v.w / v.sw
		local sy = v.h / v.sh
		if v.flip == 1 then
			tx = tx + v.w
			sx = -sx
		end

		love.graphics.setColor(sel_br, sel_br, sel_br, 1)
		love.graphics.draw(self.xml_image, v.quad, tx, ty, 0, sx, sy)

		love.graphics.setColor(sel_br * 0.9, sel_br * 0.1, sel_br * 1.0, 1)
		love.graphics.ellipse("fill", tx + v.ax * sx, ty + v.ay * sy, 0.3, 0.3)
	end

	-- Draw grid
	love.graphics.setColor(0.3, 0.3, 0.3)
	love.graphics.setBlendMode("screen", "premultiplied")
	love.graphics.draw(self.grid_image, self.grid_quad, 0, 0)
	love.graphics.setBlendMode("alpha")
	
	-- Draw selection info
	if self.select_frame ~= nil then
		local infor = string.format(" (%d,%d %dx%d)", self.select_frame.tx, self.select_frame.ty, self.select_frame.w, self.select_frame.h)
		love.graphics.setColor(0, 0, 0)
		love.graphics.print(self.select_frame.source..infor, self.select_frame.tx + 1, self.select_frame.ty - 16 + 1)
		love.graphics.setColor(1, 1, 1)
		love.graphics.print(self.select_frame.source..infor, self.select_frame.tx, self.select_frame.ty - 16)
	end
	
	-- Get coordinates
	local mx = love.mouse.getX()
	local my = love.mouse.getY()
	mx, my = love.graphics.inverseTransformPoint(mx, my)
	
	local coord = string.format("%d,%d", mx, my)
	
	love.graphics.setColor(0, 0, 0)
	love.graphics.print(coord, mx + 1, my - 16 + 1)
	love.graphics.setColor(1, 1, 1)
	love.graphics.print(coord, mx, my - 16)
end

return vram
