require 'totem'
require 'image'


local tester = totem.Tester()


local tests = {}

function tests.doesNotCrash()
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


return tester:add(tests):run()

