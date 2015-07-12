require 'totem'
require 'image'

myTests = {}
local tester = totem.Tester()

function myTests.test_ppmload()
    -- test.ppm is a 100x1 "French flag" like image, i.e the first pixel is blue
    -- the 84 next pixels are white and the 15 last pixels are red.
    -- This makes possible to implement a non regression test vs. the former
    -- PPM loader which had for effect to skip the first 85 pixels because of
    -- a header parser bug
    local img = image.load(paths.concat(sys.fpath(), "test.ppm"))
    local pix = img[{ {}, {1}, {1} }]

    -- Check the first pixel is blue
    local ref = torch.zeros(3, 1, 1); ref[3][1][1] = 1
    tester:assertTensorEq(pix, ref, 0, "PPM load: first pixel check failed")
end


function myTests.test_pgmaload()
    -- ascii.ppm is a PGMA file (ascii pgm)
    -- example comes from ehere
    -- http://people.sc.fsu.edu/~jburkardt/data/pgma/pgma.html
    local img = image.load(paths.concat(sys.fpath(), "ascii.pgm"), 1, 'byte')
    local max_gray = 15 -- 4th line of ascii.pgm
    local ascii_val = 3 -- pixel (2,2) in the file
    local pix_val = math.floor(255 * ascii_val / max_gray)

    local pix = img[{ {}, {2}, {2} }]

    -- Check that Pixel(1, 2,2) == 3
    local ref = torch.zeros(1, 1, 1)
    ref[1][1][1] = pix_val
    tester:assertTensorEq(pix, ref, 0, "PGMA load: pixel check failed")
end

function myTests.test_pgmload()
    -- test.ppm is a 100x1 "French flag" like image, i.e the first pixel is blue
    -- the 84 next pixels are white and the 15 last pixels are red.
    -- This makes possible to implement a non regression test vs. the former
    -- PPM loader which had for effect to skip the first 85 pixels because of
    -- a header parser bug
    local img = image.load(paths.concat(sys.fpath(), "test.pgm"))
    local pix = img[{ {}, {1}, {1} }]

    local ref = torch.zeros(1, 1, 1); ref[1][1][1] = 0.07
    tester:assertTensorEq(pix, ref, 0.001, "PPM load: first pixel check failed")
end

tester:add(myTests)
return tester:run(myTests)
