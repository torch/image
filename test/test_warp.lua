require 'image'
torch.setdefaulttensortype('torch.FloatTensor')
torch.setnumthreads(16)

im = image.lena()
-- Subsample lena like crazy
im = image.scale(im, im:size()[3] / 8, im:size()[2] / 8, 'bilinear')

width = im:size()[3]  -- 512 / 8
height = im:size()[2]  -- 512 / 8
nchan = im:size()[1]  -- 3
upscale = 8
width_up = width * upscale
height_up = height * upscale

-- ******************************************
-- COMPARE RESULTS OF UPSCALE (INTERPOLATION)
-- ******************************************

-- x/y grids
grid_y = torch.ger( torch.linspace(-1,1,height_up), torch.ones(width_up) )
grid_x = torch.ger( torch.ones(height_up), torch.linspace(-1,1,width_up) )

flow = torch.FloatTensor()
flow:resize(2,height_up,width_up)
flow:zero()

-- Apply scale
flow_scale = torch.FloatTensor()
flow_scale:resize(2,height_up,width_up)
flow_scale[1] = grid_y
flow_scale[2] = grid_x
flow_scale[1]:add(1):mul(0.5) -- 0 to 1
flow_scale[2]:add(1):mul(0.5) -- 0 to 1
flow_scale[1]:mul(height-1)
flow_scale[2]:mul(width-1)
flow:add(flow_scale)

t0 = sys.clock()
im_simple = image.warp(im, flow, 'simple', false)
t1 = sys.clock()
print("Upscale Time simple = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_simple, zoom = 1, legend = 'upscale simple'}

t0 = sys.clock()
im_bilinear = image.warp(im, flow, 'bilinear', false)
t1 = sys.clock()
print("Upscale Time bilinear = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_bilinear, zoom = 1, legend = 'upscale bilinear'}

t0 = sys.clock()
im_bicubic = image.warp(im, flow, 'bicubic', false)
t1 = sys.clock()
print("Upscale Time bicubic = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_bicubic, zoom = 1, legend = 'upscale bicubic'}

t0 = sys.clock()
im_lanczos = image.warp(im, flow, 'lanczos', false)
t1 = sys.clock()
print("Upscale Time lanczos = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_lanczos, zoom = 1, legend = 'upscale lanczos'}

-- *********************************************
-- NOW TRY A ROTATION AT THE STANDARD RESOLUTION
-- *********************************************

im = image.lena()
-- Subsample lena a little bit
im = image.scale(im, im:size()[3] / 4, im:size()[2] / 4, 'bilinear')

width = im:size()[3]  -- 512 / 4
height = im:size()[2]  -- 512 / 4
nchan = im:size()[1]  -- 3

grid_y = torch.ger( torch.linspace(-1,1,height), torch.ones(width) )
grid_x = torch.ger( torch.ones(height), torch.linspace(-1,1,width) )

flow = torch.FloatTensor()
flow:resize(2,height,width)
flow:zero()

-- Apply uniform scale
flow_scale = torch.FloatTensor()
flow_scale:resize(2,height,width)
flow_scale[1] = grid_y
flow_scale[2] = grid_x
flow_scale[1]:add(1):mul(0.5) -- 0 to 1
flow_scale[2]:add(1):mul(0.5) -- 0 to 1
flow_scale[1]:mul(height-1)
flow_scale[2]:mul(width-1)
flow:add(flow_scale)

flow_rot = torch.FloatTensor()
flow_rot:resize(2,height,width)
flow_rot[1] = grid_y * ((height-1)/2) * -1
flow_rot[2] = grid_x * ((width-1)/2) * -1
view = flow_rot:reshape(2,height*width)
function rmat(deg)
  local r = deg/180*math.pi
  return torch.FloatTensor{{math.cos(r), -math.sin(r)}, {math.sin(r), math.cos(r)}}
end
rot_angle = 360/7  -- a nice non-integer value
rotmat = rmat(rot_angle)
flow_rotr = torch.mm(rotmat, view)
flow_rot = flow_rot - flow_rotr:reshape( 2, height, width )
flow:add(flow_rot)

t0 = sys.clock()
im_simple = image.warp(im, flow, 'simple', false)
t1 = sys.clock()
print("Rotation Time simple = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_simple, zoom = 4, legend = 'rotation simple'}

t0 = sys.clock()
im_bilinear = image.warp(im, flow, 'bilinear', false)
t1 = sys.clock()
print("Rotation Time bilinear = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_bilinear, zoom = 4, legend = 'rotation bilinear'}

t0 = sys.clock()
im_bicubic = image.warp(im, flow, 'bicubic', false)
t1 = sys.clock()
print("Rotation Time bicubic = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_bicubic, zoom = 4, legend = 'rotation bicubic'}

t0 = sys.clock()
im_lanczos = image.warp(im, flow, 'lanczos', false)
t1 = sys.clock()
print("Rotation Time lanczos = " .. (t1 - t0))  -- Not a robust measure (should average)
image.display{image = im_lanczos, zoom = 4, legend = 'rotation lanczos'}

im_lanczos = image.warp(im, flow, 'lanczos', false, 'pad')
image.display{image = im_lanczos, zoom = 4, legend = 'rotation lanczos (default pad)'}

im_lanczos = image.warp(im, flow, 'lanczos', false, 'pad', 1)
image.display{image = im_lanczos, zoom = 4, legend = 'rotation lanczos (pad 1)'}

image.display{image = im, zoom = 4, legend = 'source image'}

-- ***********************************************
-- NOW MAKE SURE BOTH FLOAT AND DOUBLE INPUTS WORK
-- ***********************************************

mtx = torch.zeros(2, 32, 32)
out = image.warp(im:float(), mtx:float(), 'bilinear')
out = image.warp(im:double(), mtx:double(), 'bilinear')
