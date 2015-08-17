require 'image'
require 'paths'
gm = require 'graphicsmagick'

torch.setdefaulttensortype('torch.DoubleTensor')
torch.setnumthreads(4)

-- Create an instance of the test framework
local mytester = torch.Tester()
local precision = 1e-6
local test = {}

function test.CompareLoadAndDecompress()
  -- This test breaks if someone removes lena from the repo
  local imfile = '../lena.jpg'
  if not paths.filep(imfile) then
    error(imfile .. ' is missing!')
  end
  
  -- Load lena directly from the filename
  local img = image.loadJPG(imfile)
  
  -- Make sure the returned image width and height match the height and width
  -- reported by graphicsmagick (just a sanity check)
  local info = gm.info(imfile)
  local w = info.width
  local h = info.height
  mytester:assert(w == img:size(3), 'image dimension error ')
  mytester:assert(h == img:size(3), 'image dimension error ')
  
  -- Now load the raw binary from the source file into a ByteTensor
  local fin = torch.DiskFile(imfile, 'r')
  fin:binary()
  fin:seekEnd()
  local file_size_bytes = fin:position() - 1
  fin:seek(1)
  local img_binary = torch.ByteTensor(file_size_bytes)
  fin:readByte(img_binary:storage())
  fin:close()
  
  -- Now decompress the image from the ByteTensor
  local img_from_tensor = image.decompressJPG(img_binary)
  
  mytester:assertlt((img_from_tensor - img):abs():max(), precision, 
    'images from load and decompress dont match! ')
end

function test.LoadInvalid()
  -- Make sure nothing nasty happens if we try and load a "garbage" tensor
  local file_size_bytes = 1000
  local img_binary = torch.rand(file_size_bytes):mul(255):byte()
  
  -- Now decompress the image from the ByteTensor
  local ok, img_from_tensor = pcall(function()
    return image.decompressJPG(img_binary)
  end)
  
  mytester:assert(not ok or img_from_tensor == nil,
    'A non-nil was returned on an invalid input! ')
end

-- Now run the test above
mytester:add(test)
mytester:run()
