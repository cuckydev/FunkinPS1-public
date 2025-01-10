-- Modules
local slaxdom = require("slaxdom")
local nativefs = require("nativefs")
local striputf = require("striputf")

-- XML processing states
local states = {
	require("vram");
	require("sheet");
}

-- Loaded XML
local xml_name = ""
local xml = nil

-- XML processor
local state = nil

-- Viewport
local view_tx = 0
local view_ty = 0

local view_sx = 1
local view_sy = 1

-- Main program
function love.load()
	-- Setup
	love.keyboard.setKeyRepeat(true)
	love.window.setMode(1280, 720, {resizable = true})
end

local function loadXml(name)
	-- Process file name
	xml_name = name
	
	-- Read file contents
	local file = nativefs.newFile(name)
	file:open("r")
	local xml_data = striputf(file:read())
	file:close()

	-- Parse file
	xml = slaxdom:dom(xml_data)
	
	-- Find a processor that works for this xml
	for _,v in pairs(states) do
		state = v.new(xml, name)
		if state ~= nil then
			return
		end
	end
end

function love.filedropped(file)
	-- Check if extension ends with xml
	local file_name = file:getFilename():gsub("\\", "/")
	file:close()
	file = nil
	if file_name:sub(-4) ~= ".xml" then
		return
	end

	-- Load xml
	loadXml(file_name)
end

function love.keypressed(key, scancode, isrepeat)
	-- Pass to state
	if state ~= nil then
		state:keypressed(key, scancode, isrepeat)
	end
end

function love.mousepressed(x, y, button, istouch, presses)
	-- Pass to state
	if state ~= nil then
		state:mousepressed(x, y, button, istouch, presses)
	end

	-- Viewport grab
	if button == 2 then
		love.mouse.setGrabbed(true)
	end
end

function love.mousereleased(x, y, button, istouch, presses)
	-- Pass to state
	if state ~= nil then
		state:mousereleased(x, y, button, istouch, presses)
	end

	-- Viewport release
	if button == 2 then
		love.mouse.setGrabbed(false)
	end
end

function love.mousemoved(x, y, dx, dy, istouch)
	-- Pass to state
	if state ~= nil then
		state:mousemoved(x, y, dx, dy, istouch)
	end

	-- Viewport move
	if love.mouse.isGrabbed() then
		view_tx = view_tx - dx / view_sx
		view_ty = view_ty - dy / view_sy
	end
end

function love.wheelmoved(x, y)
	-- Viewport zoom
	for y = math.min(y, 0), -1 do
		view_sy = view_sy * 0.9
	end
	for y = 1, math.max(y, 0) do
		view_sy = view_sy / 0.9
	end
	view_sx = view_sy
end

function love.draw()
	-- Clear screen
	love.graphics.clear(0.3, 0.09, 0.7)
	love.graphics.setColor(1, 1, 1)

	-- Transform screen to viewport
	love.graphics.origin()

	love.graphics.translate(love.graphics.getWidth() * 0.5, love.graphics.getHeight() * 0.5)
	love.graphics.scale(view_sx, view_sy)
	love.graphics.translate(-view_tx, -view_ty)

	-- Check if XML is loaded
	if xml == nil then
		love.graphics.print("NO XML LOADED", 4, -64)
		return
	else
		love.graphics.print(xml_name, 4, -64)
	end

	if state == nil then
		love.graphics.print("XML UNIDENTIFIED", 4, -32)
	else
		state:draw()
	end
end
