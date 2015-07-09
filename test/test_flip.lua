require 'image'

torch.setdefaulttensortype('torch.DoubleTensor')
torch.setnumthreads(8)

-- Create an instance of the test framework
local precision = 1e-5
local mytester = torch.Tester()
local test = {}
local unpack = unpack or table.unpack

-- This is a correlated test (which is kinda lazy).  We're assuming HFLIP is OK.
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
      mytester:asserteq(err:abs():max(), 0, 'error - bad flip! (ndims='..
        ndims..',flip_dim='..flip_dim..')')
    end
  end
end

-- Now run the test above
mytester:add(test)
mytester:run()
