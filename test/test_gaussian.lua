require 'image'

-- Just create 2 gaussians images/kernels: 
-- - one using code copy and pasted from the old
-- - one using the c-based gaussian function
-- This test is not particularly rigorous, but the function is very simple, so
-- I'm not convinced it needs to be more complicated

sigma_horz = 0.1 + math.random() * 0.3;  -- [0.1, 0.4] 
sigma_vert = 0.1 + math.random() * 0.3;  -- [0.1, 0.4]
mean_horz = 0.1 + math.random() * 0.8;  -- [0.1, 0.9] 
mean_vert = 0.1 + math.random() * 0.8;  -- [0.1, 0.9]
width = 640
height = 480
normalize = false
amplitude = 10

im1 = image.gaussian{amplitude=amplitude, 
                     normalize=normalize, 
                     width=width, 
                     height=height, 
                     sigma_horz=sigma_horz, 
                     sigma_vert=sigma_vert, 
                     mean_horz=mean_horz, 
                     mean_vert=mean_vert}

-- The old gaussian function, commit: 71670e1dcfcfe040aba5403c800a0d316987c2ed 
function gaussian(...)
   -- process args
   local _, size, sigma, amplitude, normalize,
   width, height, sigma_horz, sigma_vert, mean_horz, mean_vert = dok.unpack(
      {...},
      'image.gaussian',
      'returns a 2D gaussian kernel',
      {arg='size', type='number', help='kernel size (size x size)', default=3},
      {arg='sigma', type='number', help='sigma (horizontal and vertical)', default=0.25},
      {arg='amplitude', type='number', help='amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='normalize kernel (exc Amplitude)', default=false},
      {arg='width', type='number', help='kernel width', defaulta='size'},
      {arg='height', type='number', help='kernel height', defaulta='size'},
      {arg='sigma_horz', type='number', help='horizontal sigma', defaulta='sigma'},
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'},
      {arg='mean_horz', type='number', help='horizontal mean', default=0.5},
      {arg='mean_vert', type='number', help='vertical mean', default=0.5}
   )

   -- local vars
   local center_x = mean_horz * width + 0.5
   local center_y = mean_vert * height + 0.5

   -- generate kernel
   local gauss = torch.Tensor(height, width)
   for i=1,height do
      for j=1,width do
         gauss[i][j] = amplitude * math.exp(-(math.pow((j-center_x)
                                                    /(sigma_horz*width),2)/2
                                           + math.pow((i-center_y)
                                                   /(sigma_vert*height),2)/2))
      end
   end
   if normalize then
      gauss:div(gauss:sum())
   end
   return gauss
end

im2 = gaussian{amplitude=amplitude,
               normalize=normalize,
               width=width,
               height=height, 
               sigma_horz=sigma_horz, 
               sigma_vert=sigma_vert, 
               mean_horz=mean_horz, 
               mean_vert=mean_vert}

-- make sure they're the same:
delta = im1:clone()
delta:mul(-1)
delta:add(im2)

print("norm(im1 - im2) = " .. delta:norm(2))

-- try one with normalization to make sure that works as well:
normalize = true
im1 = image.gaussian{amplitude=amplitude,
                     normalize=normalize,
                     width=width,
                     height=height, 
                     sigma_horz=sigma_horz, 
                     sigma_vert=sigma_vert, 
                     mean_horz=mean_horz, 
                     mean_vert=mean_vert}
im2 = gaussian{amplitude=amplitude,
               normalize=normalize,
               width=width,
               height=height,       
               sigma_horz=sigma_horz,       
               sigma_vert=sigma_vert,       
               mean_horz=mean_horz,       
               mean_vert=mean_vert}

-- make sure they're the same:
delta = im1:clone()
delta:mul(-1)
delta:add(im2)

print("norm(im1 - im2) = " .. delta:norm(2) .. " (with normalization)")

-- Now try profiling:
num_repeats = 10
time = sys.clock()
for i=1,num_repeats do
  im1 = image.gaussian{width=width, height=height}
end
time = sys.clock() - time
print("c-based gaussian with omp time: " .. time)

time = sys.clock()
for i=1,num_repeats do
  im1 = gaussian{width=width, height=height}
end
time = sys.clock() - time
print("lua-based gaussian time: " .. time)

  







