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


-- Apply all tests to both vflip and hflip
for _, flip in pairs{'vflip', 'hflip'} do
    for name, test in pairs(all_tests) do
        myTests[name .. '_' .. flip] = function() return test(flip) end
    end
end

tester:add(myTests)
return tester:run(myTests)
