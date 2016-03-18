local test = torch.TestSuite()
local precision = 1e-4
local precision_mean = 1e-3
local precision_std = 1e-1


local function getTestImagePath(name)
  return paths.concat(sys.fpath(), 'assets', name)
end


local function assertByteTensorEq(actual, expected, rcond, msg)
  rcond = rcond or 1e-5
  tester:assertTensorEq(actual:double(), expected:double(), rcond, msg)
end


local function toByteTensor(x)
  local y = torch.round(x):byte()
  y[torch.le(x, 0)] = 0
  y[torch.ge(x, 255)] = 255
  return y
end


local function toByteImage(x)
  return toByteTensor(torch.mul(x, 255))
end


local function testFunctionOnByteTensor(f, msg)
  local lena = image.lena():float()
  local expected = toByteImage(f(lena))
  local actual = f(toByteImage(lena))
  assertByteTensorEq(actual, expected, nil, msg)
end


local unpack = unpack and unpack or table.unpack -- lua52 compatibility


----------------------------------------------------------------------
-- Flip test
--
function test.FlipAgainstHFlip()
  for ndims = 1, 5 do
    for flip_dim = 1, ndims do
      local sz = {}
      for i = 1, ndims do
        sz[i] = math.random(5,10)
      end

      local input = torch.rand(unpack(sz))
      local output = image.flip(input, flip_dim)

      -- Now perform the same operation using HFLIP
      local input_tran = input
      if (flip_dim < ndims) then
        -- First permute the flip dimension to X dim
        input_tran = input:transpose(flip_dim, ndims):contiguous()
      end
      -- Now reshape it to 3D
      local original_hflip_sz = input_tran:size()
      if ndims == 1 then
        input_tran:resize(1, original_hflip_sz[1])
      end
      if ndims > 3 then
        sz1 = 1
        for i = 1, ndims - 2 do
          sz1 = sz1 * original_hflip_sz[i]
        end
        input_tran:resize(sz1, original_hflip_sz[input_tran:dim()-1],
          original_hflip_sz[input_tran:dim()])
      end

      local output_hflip = image.hflip(input_tran)

      -- Put it back to Ndim
      output_hflip:resize(original_hflip_sz)

      if (flip_dim < ndims) then
        -- permute bacx the flip dimension
        output_hflip = output_hflip:transpose(flip_dim, ndims):contiguous()
      end

      local err = output_hflip - output
      tester:asserteq(err:abs():max(), 0, 'error - bad flip! (ndims='..
        ndims..',flip_dim='..flip_dim..')')
    end
  end
end

----------------------------------------------------------------------
-- Gaussian tests
--
-- The old gaussian function, commit: 71670e1dcfcfe040aba5403c800a0d316987c2ed
local function naive_gaussian(...)
   -- process args
   local _, size, sigma, amplitude, normalize,
   width, height, sigma_horz, sigma_vert, mean_horz, mean_vert = dok.unpack(
      {...},
      'image.gaussian',
      'returns a 2D gaussian kernel',
      {arg='size', type='number', help='kernel size (size x size)', default=3},
      {arg='sigma', type='number', help='sigma (horizontal and vertical)', default=0.25},
      {arg='amplitude', type='number', help='amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='normalize kernel (exc Amplitude)', default=false},
      {arg='width', type='number', help='kernel width', defaulta='size'},
      {arg='height', type='number', help='kernel height', defaulta='size'},
      {arg='sigma_horz', type='number', help='horizontal sigma', defaulta='sigma'},
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'},
      {arg='mean_horz', type='number', help='horizontal mean', default=0.5},
      {arg='mean_vert', type='number', help='vertical mean', default=0.5}
   )

   -- local vars
   local center_x = mean_horz * width + 0.5
   local center_y = mean_vert * height + 0.5

   -- generate kernel
   local gauss = torch.Tensor(height, width)
   for i=1,height do
      for j=1,width do
         gauss[i][j] = amplitude * math.exp(-(math.pow((j-center_x)
                                                    /(sigma_horz*width),2)/2
                                           + math.pow((i-center_y)
                                                   /(sigma_vert*height),2)/2))
      end
   end
   if normalize then
      gauss:div(gauss:sum())
   end
   return gauss
end

function test.gaussian()
   local sigma_horz = 0.1 + math.random() * 0.3;  -- [0.1, 0.4]
   local sigma_vert = 0.1 + math.random() * 0.3;  -- [0.1, 0.4]
   local mean_horz = 0.1 + math.random() * 0.8;  -- [0.1, 0.9]
   local mean_vert = 0.1 + math.random() * 0.8;  -- [0.1, 0.9]
   local width = 640
   local height = 480
   local amplitude = 10

   for _, normalize in pairs{true, false} do
      im1 = image.gaussian{amplitude=amplitude,
                        normalize=normalize,
                        width=width,
                        height=height,
                        sigma_horz=sigma_horz,
                        sigma_vert=sigma_vert,
                        mean_horz=mean_horz,
                        mean_vert=mean_vert}

      im2 = naive_gaussian{amplitude=amplitude,
                  normalize=normalize,
                  width=width,
                  height=height,
                  sigma_horz=sigma_horz,
                  sigma_vert=sigma_vert,
                  mean_horz=mean_horz,
                  mean_vert=mean_vert}

      tester:assertlt(im1:add(-1, im2):sum(), precision, "Incorrect gaussian")
   end
end


function test.byteGaussian()
  local expected = toByteTensor(image.gaussian{
      amplitude = 1000,
      tensor = torch.FloatTensor(5, 5),
  })
  local actual = image.gaussian{
      amplitude = 1000,
      tensor = torch.ByteTensor(5, 5),
  }
  assertByteTensorEq(actual, expected)
end


----------------------------------------------------------------------
-- Gaussian pyramid test
--
function test.gaussianpyramid()
  -- Char, Short and Int tensors not supported.
  types = {
      'torch.ByteTensor',
      'torch.FloatTensor',
      'torch.DoubleTensor'
  }
  for _, type in ipairs(types) do
    local output = unpack(image.gaussianpyramid(torch.rand(8, 8):type(type), {0.5}))
    tester:assert(output:type() == type, 'Type ' .. type .. ' produces a different output.')
  end
end

----------------------------------------------------------------------
-- Scale test
--
local function outerProduct(x)
  x = torch.Tensor(x)
  return torch.ger(x, x)
end


function test.bilinearUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.5, 2, 3, 4, 3, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bilinear')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.bilinearDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1.25, 3, 2.5}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bilinear')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.bicubicUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.4375, 2, 3.1875, 4, 3.25, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.bicubicDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 3.1875, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.bicubicUpscale_ByteTensor()
  local im = torch.ByteTensor{{0, 1, 32}}
  local expected = torch.ByteTensor{{0, 0, 9, 32}}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  assertByteTensorEq(actual, expected)
end


function test.bilinearUpscale_ByteTensor()
  local im = torch.ByteTensor{{1, 2},
                              {2, 3}}
  local expected = torch.ByteTensor{{1, 2, 2},
                                    {2, 3, 3},
                                    {2, 3, 3}}
  local actual = image.scale(im, expected:size(2), expected:size(1))
  assertByteTensorEq(actual, expected)
end


----------------------------------------------------------------------
-- Scale test
--
local flip_tests = {}
function flip_tests.test_transformation_largeByteImage(flip)
    local x_real = image.fabio():double():mul(255)
    local x_byte = x_real:clone():byte()

    assert(x_byte:size(1) > 256 and x_byte:size(2) > 256, 'Tricky case only occurs for images larger than 256 px, pick another example')

    local f_real, f_byte
    f_real = image[flip](x_real)
    f_byte = image[flip](x_byte)
    assertByteTensorEq(f_real:byte(), f_byte, 1e-16,
        flip .. ':  result for double and byte images do not match')
end

function flip_tests.test_inplace(flip)
    local im = image.lena()
    local not_inplace = image[flip](im)
    local in_place = im:clone()
    image[flip](in_place, in_place)
    tester:assertTensorEq(in_place, not_inplace, 1e-16, flip .. ': result in-place does not match result not in-place')
end

for _, flip in pairs{'vflip', 'hflip'} do
    for name, flip_test in pairs(flip_tests) do
        test[name .. '_' .. flip] = function() return flip_test(flip) end
    end
end

function test.test_vflip_simple()
    local im_even = torch.Tensor{{1,2}, {3, 4}}
    local expected_even = torch.Tensor{{3, 4}, {1, 2}}
    local x_even = image.vflip(im_even)
    tester:assertTensorEq(expected_even, x_even, 1e-16, 'vflip: fails on even size')
    -- test inplace
    image.vflip(im_even, im_even)
    tester:assertTensorEq(expected_even, im_even, 1e-16, 'vflip: fails on even size in place')

    local im_odd = torch.Tensor{{1,2}, {3, 4}, {5, 6}}
    local expected_odd = torch.Tensor{{5,6}, {3, 4}, {1, 2}}
    local x_odd = image.vflip(im_odd)
    tester:assertTensorEq(expected_odd, x_odd, 1e-16, 'vflip: fails on odd size')
    -- test inplace
    image.vflip(im_odd, im_odd)
    tester:assertTensorEq(expected_odd, im_odd, 1e-16, 'vflip: fails on odd size in place')
end

function test.test_hflip_simple()
    local im_even = torch.Tensor{{1, 2}, {3, 4}}
    local expected_even = torch.Tensor{{2, 1}, {4, 3}}
    local x_even = image.hflip(im_even)
    tester:assertTensorEq(expected_even, x_even, 1e-16, 'hflip: fails on even size')
    -- test inplace
    image.hflip(im_even, im_even)
    tester:assertTensorEq(expected_even, im_even, 1e-16, 'hflip: fails on even size in place')

    local im_odd = torch.Tensor{{1,2, 3}, {4, 5, 6}}
    local expected_odd = torch.Tensor{{3, 2, 1}, {6, 5, 4}}
    local x_odd = image.hflip(im_odd)
    tester:assertTensorEq(expected_odd, x_odd, 1e-16, 'hflip: fails on odd size')
    -- test inplace
    image.hflip(im_odd, im_odd)
    tester:assertTensorEq(expected_odd, im_odd, 1e-16, 'hflip: fails on odd size in place')
end

----------------------------------------------------------------------
-- decompress jpg test
--
function test.CompareLoadAndDecompress()
  -- This test breaks if someone removes lena from the repo
  local imfile = getTestImagePath('lena.jpg')
  if not paths.filep(imfile) then
    error(imfile .. ' is missing!')
  end

  -- Load lena directly from the filename
  local img = image.loadJPG(imfile)

  -- Make sure the returned image width and height match the height and width
  -- reported by graphicsmagick (just a sanity check)
  local ok, gm = pcall(require, 'graphicsmagick')
  if not ok then
    -- skip this part of the test if graphicsmagick is not installed
    print('\ntest.CompareLoadAndDecompress partially requires the ' ..
          'graphicsmagick package to run. You can install it with ' ..
          '"luarocks install graphicsmagick".')
  else
    local info = gm.info(imfile)
    local w = info.width
    local h = info.height
    tester:assert(w == img:size(3), 'image dimension error ')
    tester:assert(h == img:size(3), 'image dimension error ')
  end

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

  tester:assertlt((img_from_tensor - img):abs():max(), precision,
    'images from load and decompress dont match! ')
end

function test.LoadInvalid()
  -- Make sure nothing nasty happens if we try and load a "garbage" tensor
  local file_size_bytes = 1000
  local img_binary = torch.rand(file_size_bytes):mul(255):byte()

  -- Now decompress the image from the ByteTensor
  tester:assertError(
    function() image.decompressJPG(img_binary) end,
    'A non-nil was returned on an invalid input!'
  )
end

----------------------------------------------------------------------
-- compress jpg test
--

function test.CompressAndDecompress()
  -- This test is unfortunately a correlated test: it will only be valid
  -- if decompressJPG is OK.  However, since decompressJPG has it's own unit
  -- test, this is problably fine.

  local img = image.lena()

  local quality = 100
  local img_compressed = image.compressJPG(img, quality)
  local size_100 = img_compressed:size(1)
  local img_decompressed = image.decompressJPG(img_compressed)
  local err = img_decompressed - img

  -- Now in general we will get BIG compression artifacts (even at quality=100)
  -- but they will be relatively small, so instead of a abs():max() test, we do
  -- a mean and std test.
  local mean_err = err:mean()
  local std_err = err:std()
  tester:assertlt(mean_err, precision_mean, 'compressJPG error is too high! ')
  tester:assertlt(std_err, precision_std, 'compressJPG error is too high! ')

  -- Also check that the quality setting scales the size of the compressed image
  quality = 25
  img_compressed = image.compressJPG(img, quality)
  local size_25 = img_compressed:size(1)
  tester:assertlt(size_25, size_100, 'compressJPG quality setting error! ')
end

----------------------------------------------------------------------
-- Lab conversion test
-- These tests break if someone removes lena from the repo


local function testRoundtrip(forward, backward)
  local expected = image.lena()
  local actual = backward(forward(expected))
  tester:assertTensorEq(actual, expected, 1e-4)
end


function test.rgb2lab()
  testRoundtrip(image.rgb2lab, image.lab2rgb)
end


function test.rgb2hsv()
  testRoundtrip(image.rgb2hsv, image.hsv2rgb)
end


function test.rgb2hsl()
  testRoundtrip(image.rgb2hsl, image.hsl2rgb)
end


function test.rgb2y()
  local x = torch.FloatTensor{{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}:transpose(1, 3)
  local actual = image.rgb2y(x)
  local expected = torch.FloatTensor{{{0.299}, {0.587}, {0.114}}}
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.y2jet()
  local levels = torch.Tensor{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
  local expected = image.jetColormap(10)
  local actual = image.y2jet(levels)[{{}, 1, {}}]:t()
  tester:assertTensorEq(actual, expected, 1e-5)
end


function test.rgb2labByteTensor()
  local lena = image.lena():byte()
  tester:assertError(function () image.rgb2lab(lena) end)
  tester:assertError(function () image.lab2rgb(lena) end)
end


local function testByteTensorRoundtrip(forward, backward, cond, msg)
  local lena = toByteImage(image.lena())
  local expected = lena
  local actual = backward(forward(expected))
  assertByteTensorEq(actual, expected, cond, msg)
end


function test.toFromByteTensor()
  local expected = toByteImage(image.lena():float())
  local actual = toByteImage(expected:float():div(255))
  assertByteTensorEq(actual, expected, nil, msg)
end


function test.rgb2hsvByteTensor()
  testFunctionOnByteTensor(image.rgb2hsv, 'image.rgb2hsv error for ByteTensor')
  testFunctionOnByteTensor(image.hsv2rgb, 'image.hsv2rgb error for ByteTensor')
  testByteTensorRoundtrip(image.rgb2hsv, image.hsv2rgb, 2,
                          'image.rgb2hsv roundtrip error for ByteTensor')
end


function test.rgb2hslByteTensor()
  testFunctionOnByteTensor(image.rgb2hsl, 'image.hsl2rgb error for ByteTensor')
  testFunctionOnByteTensor(image.hsl2rgb, 'image.rgb2hsl error for ByteTensor')
  testByteTensorRoundtrip(image.rgb2hsl, image.hsl2rgb, 3,
                          'image.rgb2hsl roundtrip error for ByteTensor')
end


function test.rgb2yByteTensor()
  testFunctionOnByteTensor(image.rgb2y, 'image.rgb2y error for ByteTensor')
end


function test.y2jetByteTensor()
  local levels = torch.Tensor{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
  local expected = toByteImage(image.y2jet(levels))
  local actual = image.y2jet(levels:byte())
  assertByteTensorEq(actual, expected, nil)
end


----------------------------------------------------------------------
-- PNG test
--
local function toBlob(filename)
  local f = torch.DiskFile(filename, 'r')
  f:binary()
  f:seekEnd()
  local size = f:position() - 1
  f:seek(1)
  local blob = torch.ByteTensor(size)
  f:readByte(blob:storage())
  f:close()
  return blob
end

local function checkPNG(imfile, depth, tensortype, want)
  local img = image.load(imfile, depth, tensortype)
  -- Tensors have to be converted to double, since assertTensorEq does not support ByteTensor
  --print('img: ', img)
  --print('want: ', want)
  assertByteTensorEq(img, want, precision_mean,
                    string.format('%s: pixel values are unexpected', imfile))
end

function test.LoadPNG()
  -- Gray 8-bit PNG image with width = 3, height = 1
  local gray8byte = torch.ByteTensor({{{0,127,255}}})
  checkPNG(getTestImagePath('gray3x1.png'), 1, 'byte', gray8byte)

  local gray8double = torch.DoubleTensor({{{0, 127/255, 1}}})
  checkPNG(getTestImagePath('gray3x1.png'), 1, 'double', gray8double)

  -- Gray 16-bit PNG image with width=1, height = 2
  local gray16byte = torch.ByteTensor{{{0}, {255}}}
  checkPNG(getTestImagePath('gray16-1x2.png'), 1, 'byte', gray16byte)

  local gray16float = torch.FloatTensor{{{0}, {65534/65535}}}
  checkPNG(getTestImagePath('gray16-1x2.png'), 1, 'float', gray16float)

  -- Color 8-bit PNG image with width = 2, height = 1
  local rgb8byte = torch.ByteTensor{{{255, 0}}, {{0, 127}}, {{63, 0}}}
  checkPNG(getTestImagePath('rgb2x1.png'), 3, 'byte', rgb8byte)

  local rgb8float = torch.FloatTensor{{{1, 0}}, {{0, 127/255}}, {{63/255, 0}}}
  checkPNG(getTestImagePath('rgb2x1.png'), 3, 'float', rgb8float)

  -- Color 16-bit PNG image with width = 2, height = 1
  local rgb16byte = torch.ByteTensor{{{255, 0}}, {{0, 127}}, {{63, 0}}}
  checkPNG(getTestImagePath('rgb16-2x1.png'), 3, 'byte', rgb16byte)

  local rgb16float = torch.FloatTensor{{{1, 0}}, {{0, 32767/65535}}, {{16383/65535, 0}}}
  checkPNG(getTestImagePath('rgb16-2x1.png'), 3, 'float', rgb16float)
end

function test.DecompressPNG()
  tester:assertTensorEq(
    image.load(getTestImagePath('rgb2x1.png')),
    image.decompressPNG(toBlob(getTestImagePath('rgb2x1.png'))),
    precision_mean,
    'decompressed and loaded images should be equal'
  )
end

function test.LoadCorruptedPNG()
  tester:assertErrorPattern(
    function() image.load(getTestImagePath("corrupt-ihdr.png")) end,
    "Error during init_io",
    "corrupted image should not be loaded or unexpected error message"
  )
end

----------------------------------------------------------------------
-- PPM test
--
function test.test_ppmload()
    -- test.ppm is a 100x1 "French flag" like image, i.e the first pixel is blue
    -- the 84 next pixels are white and the 15 last pixels are red.
    -- This makes possible to implement a non regression test vs. the former
    -- PPM loader which had for effect to skip the first 85 pixels because of
    -- a header parser bug
    local img = image.load(getTestImagePath("P6.ppm"))
    local pix = img[{ {}, {1}, {1} }]

    -- Check the first pixel is blue
    local ref = torch.zeros(3, 1, 1)
    ref[3][1][1] = 1
    tester:assertTensorEq(pix, ref, 0, "PPM load: first pixel check failed")
end


function test.test_pgmaload()
    -- ascii.ppm is a PGMA file (ascii pgm)
    -- example comes from ehere
    -- http://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html
    local img = image.load(getTestImagePath("P2.pgm"), 1, 'byte')
    local max_gray = 15 -- 4th line of ascii.pgm
    local ascii_val = 3 -- pixel (2,2) in the file
    local pix_val = math.floor(255 * ascii_val / max_gray)

    local pix = img[1][2][2]

    -- Check that Pixel(1, 2,2) == 3
    local ref = pix_val
    tester:asserteq(pix, ref, "PGMA load: pixel check failed")
end

function test.test_pgmload()
    -- test.ppm is a 100x1 "French flag" like image, i.e the first pixel is blue
    -- the 84 next pixels are white and the 15 last pixels are red.
    -- This makes possible to implement a non regression test vs. the former
    -- PPM loader which had for effect to skip the first 85 pixels because of
    -- a header parser bug
    local img = image.load(getTestImagePath("P5.pgm"))
    local pix = img[{ {}, {1}, {1} }]

    local ref = torch.zeros(1, 1, 1); ref[1][1][1] = 0.07
    tester:assertTensorEq(pix, ref, 0.001, "PPM load: first pixel check failed")
end

function test.test_pbmload()
  -- test.pbm is a Portable BitMap (not supported)
  tester:assertErrorPattern(
    function() image.loadPPM(getTestImagePath("P4.pbm")) end,
    "unsupported magic number",
    "PBM format should not be loaded or unexpected error message"
  )
end

----------------------------------------------------------------------
-- Text drawing test
--
function test.test_textdraw()
  local types = {
     ["torch.ByteTensor"]   = "byte",
     ["torch.DoubleTensor"] = "double",
     ["torch.FloatTensor"]  = "float"
  }
  for k,v in pairs(types) do
    local img = image.drawText(
       torch.zeros(3, 24, 24):type(k),
       "foo\nbar", 2, 4, {color={255, 255, 255}, bg={255, 0, 0}}
    )
    checkPNG(getTestImagePath("foobar.png"), 3, v, img)
  end
end


function image.test(tests, seed)
   local defaultTensorType = torch.getdefaulttensortype()
   torch.setdefaulttensortype('torch.DoubleTensor')
   seed = seed or os.time()
   print('seed: ', seed)
   math.randomseed(seed)
   tester = torch.Tester()
   tester:add(test)
   tester:run(tests)
   torch.setdefaulttensortype(defaultTensorType)
   return tester
end
