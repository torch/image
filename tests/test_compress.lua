require 'torch'
require 'image'

local tester = torch.Tester()
local compresstests = {}

local function maxdiff(x,y)
   local d = x-y
   if x:type() == 'torch.DoubleTensor' or x:type() == 'torch.FloatTensor' then
      return d:abs():max()
   else
      local dd = torch.Tensor():resize(d:size()):copy(d)
      return dd:abs():max()
   end
end

local im = image.load('lena.png', nil, "byte")
local size = im:size()
local png, decompress_f = image.compress(im)

function compresstests.decompress_inplace()
    local decompressed = torch.ByteTensor(size)
    image.decompress(png, decompressed)
    tester:asserteq(maxdiff(im, decompressed), 0, 'decompressing into a tensor does not give the original tensor!')
end

function compresstests.decompress_given_size()
    local decompressed = image.decompress(png, size)
    tester:asserteq(maxdiff(im, decompressed), 0, 'decompressing given a size does not give the original tensor!')
end

function compresstests.decompress_via_closure()
    local decompressed = decompress_f()
    tester:asserteq(maxdiff(im, decompressed), 0, 'decompressing via closure does not give the original tensor!')
end

function compresstests.noncontiguous()
    local decompressed = torch.ByteTensor(size):transpose(2,3):clone():transpose(3,2)
    should_error = function() image.decompress(png, decompressed) end
    tester:assertError(should_error, 'Attempting to decompress into a noncontiguous tensor should error')
end

function compresstests.correctsize()
    local decompressed = torch.ByteTensor(size):select(1,1):contiguous()
    should_error = function() image.decompress(png, decompressed) end
    tester:assertError(should_error, 'Attempting to decompress into a tensor of the wrong size should error')
end

tester:add(compresstests)
tester:run()
