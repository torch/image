require 'image'
require 'paths'

-- Create an instance of the test framework
local mytester = torch.Tester()
local precision = 1e-4
local test = {}

function test.TestLabConversionBackAndForth()
  -- This test breaks if someone removes lena from the repo
  local imfile = '../lena.jpg'
  if not paths.filep(imfile) then
    error(imfile .. ' is missing!')
  end

  -- Load lena directly from the filename
  local img = image.loadJPG(imfile)

  -- Convert to LAB and back to RGB
  local lab = image.rgb2lab(img)
  local img2 = image.lab2rgb(lab)
  -- Compare RGB images
  mytester:assertlt((img - img2):abs():max(), precision,
    'RGB <-> LAB conversion produces wrong results! ')
end

-- Now run the test above
mytester:add(test)
mytester:run()
