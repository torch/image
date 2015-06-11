require 'image'
require 'paths'

torch.setdefaulttensortype('torch.DoubleTensor')
torch.setnumthreads(4)

-- Create an instance of the test framework
local mytester = torch.Tester()
local precision_mean = 1e-3
local precision_std = 1e-1
local test = {}

function checkPNG(imfile, depth, tensortype, want)
  local img = image.load(imfile, 1, tensortype)
  -- Tensors have to be converted to double, since assertTensorEq does not support ByteTensor
  mytester:assertTensorEq(img:double(), want:double(), precision_mean, string.format('%s: pixel values are unexpected', imfile))
end

function test.LoadPNG()
  -- Gray 8-bit PNG image with width = 3, height = 1
  local gray8byte = torch.ByteTensor(1, 1, 3)
  gray8byte[1][1][1] = 0
  gray8byte[1][1][2] = 127
  gray8byte[1][1][3] = 255
  checkPNG('gray3x1.png', 1, 'byte', gray8byte)

  local gray8double = torch.DoubleTensor(1, 1, 3)
  gray8double[1][1][1] = 0
  gray8double[1][1][2] = 127 / 255
  gray8double[1][1][3] = 1
  checkPNG('gray3x1.png', 1, 'double', gray8double)


  -- Gray 16-bit PNG image with width=1, height = 2
  local gray16byte = torch.ByteTensor(1, 2, 1)
  gray16byte[1][1][1] = 0
  gray16byte[1][2][1] = 255
  checkPNG('gray16-1x2.png', 1, 'byte', gray16byte)

  local gray16float = torch.FloatTensor(1, 2, 1)
  gray16float[1][1][1] = 0
  gray16float[1][2][1] = 65534 / 65535
  checkPNG('gray16-1x2.png', 1, 'float', gray16float)
  
end

-- Now run the test above
mytester:add(test)
mytester:run()
