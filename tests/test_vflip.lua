require 'totem'
require 'image'

myTests = {}
local tester = totem.Tester()

local function test_transformation(f)
    local x_real = image.fabio():double():mul(255)
    local x_byte = x_real:clone():byte()

    assert(x_byte:size(1) > 256 and x_byte:size(2) > 256, 'Tricky case only occurs for images larger than 256 px, pick another example')

    local f_real, f_byte
    f_real = image[f](x_real)
    f_byte = image[f](x_byte)
    tester:assertTensorEq(f_real:byte():double(), f_byte:double(), 1e-16, f .. ':  result for double and byte images do not match')
end


function myTests.test_vflip()
    test_transformation('vflip')
end


function myTests.test_hflip()
    test_transformation('hflip')
end


tester:add(myTests)
return tester:run(myTests)
