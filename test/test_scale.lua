require 'image'
require 'torch'


torch.setdefaulttensortype('torch.FloatTensor')

local tester = torch.Tester()
local tests = {}


local function outerProduct(x)
  x = torch.Tensor(x)
  return torch.ger(x, x)
end


local function assertTensorEq(actual, expected)
  if torch.type(expected) == 'torch.ByteTensor' then
    local areEqual = torch.eq(actual, expected):all()
    tester:assert(areEqual)
  else
    tester:assertTensorEq(actual, expected, 1e-5)
  end
end


function tests.bilinearUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.5, 2, 3, 4, 3, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bilinear')
  assertTensorEq(actual, expected)
end


function tests.bilinearDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1.25, 3, 2.5}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bilinear')
  assertTensorEq(actual, expected)
end


function tests.bicubicUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.4375, 2, 3.1875, 4, 3.25, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  assertTensorEq(actual, expected)
end


function tests.bicubicDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 3.1875, 2}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  assertTensorEq(actual, expected)
end


function tests.bicubicUpscale_ByteTensor()
  local im = torch.ByteTensor{{0, 1, 32}}
  local expected = torch.ByteTensor{{0, 0, 9, 32}}
  local actual = image.scale(im, expected:size(2), expected:size(1), 'bicubic')
  assertTensorEq(actual, expected)
end


function tests.bilinearUpscale_ByteTensor()
  local im = torch.ByteTensor{{1, 2},
                              {2, 3}}
  local expected = torch.ByteTensor{{1, 1, 2},
                                    {1, 1, 2},
                                    {2, 2, 3}}
  local actual = image.scale(im, expected:size(2), expected:size(1))
  assertTensorEq(actual, expected)
end


tester:add(tests)
tester:run()

