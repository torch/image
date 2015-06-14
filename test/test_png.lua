require 'image'

-- Create an instance of the test framework
local mytester = torch.Tester()
local precision_mean = 1e-3
local test = {}

function checkPNG(imfile, depth, tensortype, want)
  local img = image.load(imfile, depth, tensortype)
  -- Tensors have to be converted to double, since assertTensorEq does not support ByteTensor
  print('img: ', img)
  print('want: ', want)
  mytester:assertTensorEq(img:double(), want:double(), precision_mean,
                          string.format('%s: pixel values are unexpected', imfile))
end

function test.LoadPNG()
  -- Gray 8-bit PNG image with width = 3, height = 1
  local gray8byte = torch.ByteTensor({{{0,127,255}}})
  checkPNG('gray3x1.png', 1, 'byte', gray8byte)

  local gray8double = torch.DoubleTensor({{{0, 127/255, 1}}})
  checkPNG('gray3x1.png', 1, 'double', gray8double)

  -- Gray 16-bit PNG image with width=1, height = 2
  local gray16byte = torch.ByteTensor({{{0, 255}}})
  checkPNG('gray16-1x2.png', 1, 'byte', gray16byte)

  local gray16float = torch.FloatTensor({{{0, 65534/65535}}})
  checkPNG('gray16-1x2.png', 1, 'float', gray16float)

  -- Color 8-bit PNG image with width = 2, height = 1
  local rgb8byte = torch.ByteTensor({{{255, 0, 0, 127, 63, 0}}})
  checkPNG('rgb2x1.png', 3, 'byte', rgb8byte)

  local rgb8float = torch.FloatTensor({{{1, 0, 0, 127/255, 63/255, 0}}})
  checkPNG('rgb2x1.png', 3, 'float', rgb8float)

  -- Color 16-bit PNG image with width = 2, height = 1
  local rgb16byte = torch.ByteTensor({{{255, 0, 0, 127, 63, 0}}})
  checkPNG('rgb16-2x1.png', 3, 'byte', rgb16byte)

  local rgb16float = torch.FloatTensor({{{1, 0, 0, 32767/65535, 16383/65535, 0}}})
  checkPNG('rgb16-2x1.png', 3, 'float', rgb16float)
end

-- Now run the test above
mytester:add(test)
mytester:run()
