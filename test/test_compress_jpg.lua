require 'image'
require 'paths'

torch.setdefaulttensortype('torch.DoubleTensor')
torch.setnumthreads(4)

-- Create an instance of the test framework
local mytester = torch.Tester()
local precision_mean = 1e-3
local precision_std = 1e-1
local test = {}

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
  mytester:assertlt(mean_err, precision_mean, 'compressJPG error is too high! ')
  mytester:assertlt(std_err, precision_std, 'compressJPG error is too high! ')

  -- Also check that the quality setting scales the size of the compressed image
  quality = 25
  img_compressed = image.compressJPG(img, quality)
  local size_25 = img_compressed:size(1)
  mytester:assertlt(size_25, size_100, 'compressJPG quality setting error! ')
end

-- Now run the test above
mytester:add(test)
mytester:run()
