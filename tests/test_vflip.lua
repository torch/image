require 'totem'
require 'image'

myTests = {}
local tester = totem.Tester()


-- List of all possible tests
local all_tests = {}

function all_tests.test_transformation_largeByteImage(flip)
    local x_real = image.fabio():double():mul(255)
    local x_byte = x_real:clone():byte()

    assert(x_byte:size(1) > 256 and x_byte:size(2) > 256, 'Tricky case only occurs for images larger than 256 px, pick another example')

    local f_real, f_byte
    f_real = image[flip](x_real)
    f_byte = image[flip](x_byte)
    tester:assertTensorEq(f_real:byte():double(), f_byte:double(), 1e-16, flip .. ':  result for double and byte images do not match')
end

function all_tests.test_inplace(flip)
    local im = image.lena()
    local not_inplace = image[flip](im)
    local in_place = im:clone()
    image[flip](in_place, in_place)
    tester:assertTensorEq(in_place, not_inplace, 1e-16, flip .. ': result in-place does not match result not in-place')
end


-- Apply all tests to both vflip and hflip
for _, flip in pairs{'vflip', 'hflip'} do
    for name, test in pairs(all_tests) do
        myTests[name .. '_' .. flip] = function() return test(flip) end
    end
end

function myTests.test_vflip_simple()
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

function myTests.test_hflip_simple()
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


tester:add(myTests)
return tester:run(myTests)
