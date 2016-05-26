require 'image'

torch.setdefaulttensortype('torch.FloatTensor')
torch.setnumthreads(16)

local function test_rotate(src, mode)
   torch.manualSeed(11)
   local mean_dist = 0.0
   for i = 1, 10 do
      local theta = torch.uniform(0, 2 * math.pi)
      local d1, d2, d3, d4
      
      -- rotate
      if mode then
         d1 = image.rotate(src, theta, mode)
         d2 = src.new():resizeAs(src)
         image.rotate(d2, src, theta, mode)
      else
         d1 = image.rotate(src, theta)
         d2 = src.new():resizeAs(src)
         image.rotate(d2, src, theta)
      end

      -- revert
      local revert = 2 * math.pi - theta
      if mode then
         d3 = image.rotate(d1, revert, mode)
         d4 = src.new():resizeAs(src)
         image.rotate(d4, d2, revert, mode)
      else
         d3 = image.rotate(d1, revert)
         d4 = src.new():resizeAs(src)
         image.rotate(d4, d2, revert)
      end
      
      -- diff
      if src:dim() == 3 then
         local cs = image.crop(src, src:size(2) / 4, src:size(3) / 4, src:size(2) / 4 * 3, src:size(3) / 4 * 3)
         local c3 = image.crop(d3, src:size(2) / 4, src:size(3) / 4, src:size(2) / 4 * 3, src:size(3) / 4 * 3)
         local c4 = image.crop(d4, src:size(2) / 4, src:size(3) / 4, src:size(2) / 4 * 3, src:size(3) / 4 * 3)
         mean_dist = mean_dist + cs:dist(c3)
         mean_dist = mean_dist + cs:dist(c4)
      elseif src:dim() == 2 then
         local cs = image.crop(src, src:size(1) / 4, src:size(2) / 4, src:size(1) / 4 * 3, src:size(2) / 4 * 3)
         local c3 = image.crop(d3, src:size(1) / 4, src:size(2) / 4, src:size(1) / 4 * 3, src:size(2) / 4 * 3)
         local c4 = image.crop(d4, src:size(1) / 4, src:size(2) / 4, src:size(1) / 4 * 3, src:size(2) / 4 * 3)
         mean_dist = mean_dist + cs:dist(c3)
         mean_dist = mean_dist + cs:dist(c4)
      end
      --[[
      if i == 1 then
         image.display(src)
         image.display(d1)
         image.display(d2)
         image.display(d3)
         image.display(d4)
      end
      --]]
   end
   if mode then
      print("mode = " .. mode .. ", mean dist: " .. mean_dist / (10 * 2))
   else
      print("mode = nil, mean dist: " .. mean_dist / (10 * 2))
   end
end
local src = image.scale(image.lena(), 128, 128, 'bilinear')
print("** dim3")
test_rotate(src, nil)
test_rotate(src, 'simple')
test_rotate(src, 'bilinear')
print("** dim2")
src = src:select(1, 1)
test_rotate(src, nil)
test_rotate(src, 'simple')
test_rotate(src, 'bilinear')
