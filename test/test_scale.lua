require 'image'
require 'torch'


torch.setdefaulttensortype('torch.FloatTensor')

local tester = torch.Tester()
local tests = {}


local function outerProduct(x)
  x = torch.Tensor(x)
  return torch.ger(x, x)
end


function tests.bilinearUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.5, 2, 3, 4, 3, 2}
  local actual = image.scale(im, expected:size(1), expected:size(2), 'bilinear')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function tests.bilinearDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1.25, 3, 2.5}
  local actual = image.scale(im, expected:size(1), expected:size(2), 'bilinear')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function tests.bicubicUpscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 1.4375, 2, 3.1875, 4, 3.25, 2}
  local actual = image.scale(im, expected:size(1), expected:size(2), 'bicubic')
  tester:assertTensorEq(actual, expected, 1e-5)
end


function tests.bicubicDownscale()
  local im = outerProduct{1, 2, 4, 2}
  local expected = outerProduct{1, 3.1875, 2}
  local actual = image.scale(im, expected:size(1), expected:size(2), 'bicubic')
  tester:assertTensorEq(actual, expected, 1e-5)
end


tester:add(tests)
tester:run()

