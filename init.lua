----------------------------------------------------------------------
--
-- Copyright (c) 2011 Ronan Collobert, Clement Farabet
-- 
-- Permission is hereby granted, free of charge, to any person obtaining
-- a copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, sublicense, and/or sell copies of the Software, and to
-- permit persons to whom the Software is furnished to do so, subject to
-- the following conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
-- LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
-- OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
-- WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-- 
----------------------------------------------------------------------
-- description:
--     image - an image toolBox, for Torch
--
-- history: 
--     July  1, 2011, 7:42PM - import from Torch5 - Clement Farabet
----------------------------------------------------------------------

require 'torch'
require 'sys'
require 'xlua'
require 'libimage'

----------------------------------------------------------------------
-- save/load in multiple formats
--
local function loadPNG(filename, depth)
   if not xlua.require 'libpng' then
      xlua.error('libpng package not found, please install libpng','image.loadPNG')
   end
   local MAXVAL = 255
   local a = torch.Tensor().libpng.load(filename)
   a:mul(1/MAXVAL)
   if depth and depth == 1 then
      if a:nDimension() == 2 then
         -- all good
      elseif a:size(3) == 3 then
         local b = torch.Tensor(a:size(1), a:size(2))
         image.rgb2y(a,b)
         a = b
      elseif a:size(3) ~= 1 then
         xlua.error('image loaded has wrong #channels','image.loadPNG')
      end
   elseif depth and depth == 3 then
      if a:size(1) ~= 3 then
         xlua.error('image loaded has wrong #channels','image.loadPNG')
      end
   end
   return a
end
rawset(image, 'loadPNG', loadPNG)

local function savePNG(filename, tensor)
   if not xlua.require 'libpng' then
      xlua.error('libpng package not found, please install libpng','image.savePNG')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resizeAs(tensor):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libpng.save(filename, a)
end  
rawset(image, 'savePNG', savePNG)

local function loadJPG(filename, depth)
   if not xlua.require 'libjpeg' then
      xlua.error('libjpeg package not found, please install libjpeg','image.loadJPG')
   end
   local MAXVAL = 255
   local a = torch.Tensor().libjpeg.load(filename)
   a:mul(1/MAXVAL)
   if depth and depth == 1 then
      if a:nDimension() == 2 then
         -- all good
      elseif a:size(3) == 3 then
         local b = torch.Tensor(a:size(1), a:size(2))
         image.rgb2y(a,b)
         a = b
      elseif a:size(3) ~= 1 then
         xlua.error('image loaded has wrong #channels','image.loadJPG')
      end
   elseif depth and depth == 3 then
      if a:size(1) ~= 3 then
         xlua.error('image loaded has wrong #channels','image.loadJPG')
      end
   end
   return a
end
rawset(image, 'loadJPG', loadJPG)

local function saveJPG(filename, tensor)
   if not xlua.require 'libjpeg' then
      xlua.error('libjpeg package not found, please install libjpeg','image.saveJPG')
   end
   local MAXVAL = 255
   local a = torch.Tensor():resizeAs(tensor):copy(tensor)
   a.image.saturate(a) -- bound btwn 0 and 1
   a:mul(MAXVAL)       -- remap to [0..255]
   a.libjpeg.save(filename, a)
end
rawset(image, 'saveJPG', saveJPG)

function image.getJPGsize(filename)
   if not xlua.require 'libjpeg' then
      xlua.error('libjpeg package not found, please install libjpeg','image.loadJPG')
   end
   return torch.Tensor().libjpeg.size(filename)
end
rawset(image, 'getJPGsize', getJPGsize)

local function load(filename, depth)
   local ext = string.match(filename,'%.(%a+)$')
   local tensor
   if ext == 'jpg' or ext == 'JPG' then
      tensor = image.loadJPG(filename,depth)
   elseif ext == 'png' or ext == 'PNG' then
      tensor = image.loadPNG(filename,depth)
   else
      xlua.error('unknown image type: ' .. ext, 'image.load')
   end
   return tensor
end
rawset(image, 'load', load)

local function save(filename, tensor)
   local ext = string.match(filename,'%.(%a+)$')
   if ext == 'jpg' or ext == 'JPG' then
      image.saveJPG(filename, tensor)
   elseif ext == 'png' or ext == 'PNG' then
      image.savePNG(filename, tensor)
   else
      xlua.error('unknown image type: ' .. ext, 'image.save')
   end
end
rawset(image, 'save', save)

----------------------------------------------------------------------
-- crop
--
local function crop(src,dst,startx,starty,endx,endy)
   xlua.error('not adapted to Torch7 yet', 'image.crop')
   if endx==nil then
      return src.image.cropNoScale(src,dst,startx,starty);
   else
      local depth=0;
      if src:nDimension()>2 then
         depth=src:size(3);
      end
      local x=torch.Tensor(endx-startx,endy-starty,depth);
      src.image.cropNoScale(src,x,startx,starty);
      image.scale(x,dst);
   end
end
rawset(image, 'crop', crop)

----------------------------------------------------------------------
-- scale
--
local function scale(src,dst,type)
   if not type or type=='bilinear' then
      src.image.scaleBilinear(src,dst)
   elseif type=='simple' then
      xlua.error('not adapted to Torch7 yet', 'image.scaleSimple')
      src.image.scaleSimple(src,dst)
   else
      xlua.error('type must be one of: simple | bilinear', 'image.scale')
   end
   
end
rawset(image, 'scale', scale)

----------------------------------------------------------------------
-- translate
--
local function translate(src,dst,x,y)
   xlua.error('not adapted to Torch7 yet', 'image.translate')
   src.image.translate(src,dst,x,y);   
end
rawset(image, 'translate', translate)

----------------------------------------------------------------------
-- rotate
--
local function rotate(src,dst,theta)
   xlua.error('not adapted to Torch7 yet', 'image.rotate')
   src.image.rotate(src,dst,theta);   
end
rawset(image, 'rotate', rotate)

----------------------------------------------------------------------
-- convolve routine
--
local function convolveInPlace(mysrc,kernel,pad_const)
   local kH=kernel:size(1);  
   local kW=kernel:size(2); 
   local stepW=1; 
   local stepH=1;  
   
   local inputHeight =mysrc:size(1); 
   local outputHeight = (inputHeight-kH)/stepH + 1
   local inputWidth = mysrc:size(2);  
   local outputWidth = (inputWidth-kW)/stepW + 1

   -- create destination so it is the same size as input,
   -- and pad input so convolution makes the same size
   outputHeight=inputHeight; 
   outputWidth=inputWidth;
   inputWidth=((outputWidth-1)*stepW)+kW; 
   inputHeight=((outputHeight-1)*stepH)+kH; 
   local src;
   src=torch.Tensor(inputHeight,inputWidth);
   src:zero(); src=src + pad_const;
   src.image.translate(mysrc,src,math.floor(kW/2),math.floor(kH/2));
   
   mysrc:zero(); 

   mysrc:addT4dotT2(1,
                    src:unfold(2, kW, stepW):unfold(1, kH, stepH),
                    kernel)
   return mysrc;
end 
rawset(image, 'convolveInPlace', convolveInPlace) 

----------------------------------------------------------------------
-- convolve dest
--
local function convolveToDst(src,dst,kernel)
   local kH=kernel:size(1);  
   local kW=kernel:size(2); 
   local stepW=1; 
   local stepH=1; 
   
   local inputHeight =src:size(1); 
   local outputHeight = (inputHeight-kH)/stepH + 1
   local inputWidth = src:size(2);  
   local outputWidth = (inputWidth-kW)/stepW + 1
   
   if dst==nil then
      dst=torch.Tensor(outputHeight,outputWidth); 
      dst:zero();
   end  
   
   dst:addT4dotT2(1,
                  src:unfold(2, kW, stepW):unfold(1, kH, stepH),
                  kernel)
   return dst;
end 
rawset(image, 'convolveToDst', convolveToDst) 

----------------------------------------------------------------------
-- main convolve
--
local function convolve(p1,p2,p3)
   if type(p3)=="number" then
      image.convolveInPlace(p1,p2,p3)
   else
      image.convolveToDst(p1,p2,p3)
   end
end
rawset(image, 'convolve', convolve) 

----------------------------------------------------------------------
-- compresses an image between min and max
--
local function minmax(args)
   local tensor = args.tensor
   local min = args.min
   local max = args.max 
   local inplace = args.inplace or false
   local tensorOut = args.tensorOut or (inplace and tensor) 
      or torch.Tensor(tensor:size()):copy(tensor)

   -- resize
   if args.tensorOut then
      tensorOut:resizeAs(tensor):copy(tensor)
   end

   -- rescale min
   if (min == nil) then
      min = tensorOut:min()
   end
   if (min ~= 0) then tensorOut:add(-min) end

   -- rescale for max
   if (max == nil) then
      max = tensorOut:max()
   else
      max = max - min
   end
   if (max ~= 0) then tensorOut:div(max) end
      
   -- saturate
   tensorOut.image.saturate(tensorOut)

   -- and return
   return tensorOut
end
rawset(image, 'minmax', minmax) 

----------------------------------------------------------------------
-- super generic display function
--
local function display(...)
   -- usage
   local _, input, zoom, min, max, legend, w, gui = xlua.unpack(
      {...},
      'image.display',
      'displays a single image, with optional saturation/zoom',
      {arg='image', type='torch.Tensor', help='image, (1xHxW or 3xHxW or HxW)', req=true},
      {arg='zoom', type='number', help='display zoom', default=1},
      {arg='min', type='number', help='lower-bound for range'},
      {arg='max', type='number', help='upper-bound for range'},
      {arg='legend', type='string', help='legend', default='image.display'},
      {arg='win', type='gfx.Window', help='window descriptor'},
      {arg='gui', type='boolean', help='if on, user can zoom in/out (turn off for faster display)',
       default=true}
   )

   -- dependencies
   require 'qt'
   require 'qttorch'
   require 'qtwidget'
   require 'qtuiloader'

   -- if single image, blit, else transfer over to displayList
   if input:nDimension() == 2 
   or (input:nDimension() == 3 and (input:size(1) == 1 or input:size(1) == 3)) then
      -- Rescale range
      local mminput = image.minmax{tensor=input, min=min, max=max}

      -- Compute width
      local d = input:nDimension()
      local x = wx or input:size(d)*zoom
      local y = wy or input:size(d-1)*zoom

      -- if gui active, then create interactive window (with zoom, clicks and so on)
      if gui and not w then
         -- create window context
         local closure = w
         local hook_resize, hook_mouse
         if closure and closure.window and closure.image then
            closure.image = mminput
            closure.refresh(x,y)
         else
            closure = {image=mminput}
            hook_resize = function(wi,he)
                             local qtimg = qt.QImage.fromTensor(closure.image)
                             closure.painter:image(0,0,wi,he,qtimg)
                             collectgarbage()
                          end
            hook_mouse = function(x,y,button)
                            local size = closure.window.frame.size:totable()
                            if button == 'LeftButton' then
                               size.width = size.width * 1.2
                                  size.height = size.height * 1.2
                            elseif button == 'RightButton' then
                               size.width = size.width / 1.2
                               size.height = size.height / 1.2
                            end
                            closure.window.frame.size = qt.QSize(size)
                         end
            closure.window, closure.painter = image.window(hook_resize,hook_mouse)
            closure.refresh = hook_resize
         end
         closure.window.size = qt.QSize{width=x,height=y}
         closure.window.windowTitle = legend
         hook_resize(x,y)
         closure.window:show()
         closure.isclosure = true
         return closure
      else
         w = w or qtwidget.newwindow(x,y,legend)
         if w.isclosure then
            -- window was created with gui, just update closure
            local closure = w
            closure.image = mminput
            closure.window.size = qt.QSize{width=x,height=y}
            closure.window.windowTitle = legend
            closure.refresh(x,y)
         else
            -- if no gui, create plain window, and blit
            local qtimg = qt.QImage.fromTensor(mminput)
            w:image(0,0,x*zoom,y*zoom,qtimg)
         end
      end
   else 
      xerror('only supports 1 or 3 channels','image.display')
   end
   -- return painter
   return w
end
rawset(image, 'display', display)

----------------------------------------------------------------------
-- creates a window context for images
--
local function window(hook_resize, hook_mousepress, hook_mousedoublepress)
   local pathui = sys.concat(sys.fpath(), 'win.ui')
   local win = qtuiloader.load(pathui)
   local painter = qt.QtLuaPainter(win.frame)
   if hook_resize then
      qt.connect(qt.QtLuaListener(win.frame), 
                 'sigResize(int,int)', 
                 hook_resize)
   end
   if hook_mousepress then
      qt.connect(qt.QtLuaListener(win.frame),
                 'sigMousePress(int,int,QByteArray,QByteArray,QByteArray)', 
                 hook_mousepress)
   end
   if hook_mousedoublepress then
      qt.connect(qt.QtLuaListener(win.frame),
                 'sigMouseDoubleClick(int,int,QByteArray,QByteArray,QByteArray)',
                 hook_mousedoublepress)
   end
   local ctrl = false
   qt.connect(qt.QtLuaListener(win),
              'sigKeyPress(QString,QByteArray,QByteArray)',
              function (str, s2)
                 if s2 and s2 == 'Key_Control' then
                    ctrl = true
                 elseif s2 and s2 == 'Key_W' and ctrl then
                    win:close()
                 else
                    ctrl = false
                 end
              end)
   return win,painter
end
rawset(image, 'window', window)

----------------------------------------------------------------------
-- lena is always useful
--
local function lena()
   if xlua.require 'libpng' then
      lena = image.load(sys.concat(sys.fpath(), 'lena.png'), 3)
   elseif xlua.require 'libjpg' then
      lena = image.load(sys.concat(sys.fpath(), 'lena.jpg'), 3)
   else
      xlua.error('no bindings available to load images (libjpeg AND libpng missing)', 'image.lena')
   end
   return lena
end
rawset(image, 'lena', lena)

----------------------------------------------------------------------
-- image.rgb2yuv(image)
-- converts a RGB image to YUV
--
function image.rgb2yuv(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize
   output:resizeAs(input)
   
   -- input chanels
   local inputRed = input[1]
   local inputGreen = input[2]
   local inputBlue = input[3]
   
   -- output chanels
   local outputY = output[1]
   local outputU = output[2]
   local outputV = output[3]
   
   -- convert
   outputY:copy( inputRed*0.299 + inputGreen*0.587 + inputBlue*0.114 )
   outputU:copy( inputRed*(-0.14713) 
              + inputGreen*(-0.28886) 
           + inputBlue*0.436 )
   outputV:copy( inputRed*0.615 
                 + inputGreen*(-0.51499) 
              + inputBlue*(-0.10001) )
   
   -- return YUV image
   return output
end

----------------------------------------------------------------------
-- image.yuv2rgb(image)
-- converts a YUV image to RGB
--
function image.yuv2rgb(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize
   output:resizeAs(input)
   
   -- input chanels
   local inputY = input[1]
   local inputU = input[2]
   local inputV = input[3]
   
   -- output chanels
   local outputRed = output[1]
   local outputGreen = output[2]
   local outputBlue = output[3]
   
   -- convert
   outputRed:copy(inputY):add(1.13983, inputV)
   outputGreen:copy(inputY):add(-0.39465, inputU):add(-0.58060, inputV)      
   outputBlue:copy(inputY):add(2.03211, inputU)
   
   -- return RGB image
   return output
end

----------------------------------------------------------------------
-- image.rgb2y(image)
-- converts a RGB image to Y (discards U/V)
--
function image.rgb2y(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize
   output:resize(1, input:size(2), input:size(3))
   
   -- input chanels
   local inputRed = input[1]
   local inputGreen = input[2]
   local inputBlue = input[3]
   
   -- output chanels
   local outputY = output[1]
   
   -- convert
   outputY:zero():add(0.299, inputRed):add(0.587, inputGreen):add(0.114, inputBlue)
   
   -- return YUV image
   return output
end

----------------------------------------------------------------------
-- image.rgb2hsl(image)
-- converts an RGB image to HSL
--
function image.rgb2hsl(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize and compute
   output:resizeAs(input)
   input.image.rgb2hsl(input,output)
   
   -- return HSL image
   return output
end

----------------------------------------------------------------------
-- image.hsl2rgb(image)
-- converts an HSL image to RGB
--
function image.hsl2rgb(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize and compute
   output:resizeAs(input)
   input.image.hsl2rgb(input,output)
   
   -- return HSL image
   return output
end

----------------------------------------------------------------------
-- image.rgb2hsv(image)
-- converts an RGB image to HSV
--
function image.rgb2hsv(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize and compute
   output:resizeAs(input)
   input.image.rgb2hsv(input,output)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
-- image.hsv2rgb(image)
-- converts an HSV image to RGB
--
function image.hsv2rgb(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   
   -- resize and compute
   output:resizeAs(input)
   input.image.hsv2rgb(input,output)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
-- image.rgb2nrgb(image)
-- converts an RGB image to normalized-RGB
--
function image.rgb2nrgb(input, ...)
   local arg = {...}
   local output = arg[1] or torch.Tensor()
   local sum = torch.Tensor()
   
   -- resize tensors
   output:resizeAs(input)
   sum:resize(input:size(2), input:size(3))
   
   -- compute sum and normalize
   sum:copy(input[1]):add(input[2]):add(input[3]):add(1e-6)
   output:copy(input)
   output[1]:cdiv(sum)
   output[2]:cdiv(sum)
   output[3]:cdiv(sum)
   
   -- return HSV image
   return output
end

----------------------------------------------------------------------
--- Returns a gaussian The.
-- kernel default parameters generate that occupies the entire kernel.
--
function image.gaussian(...)
   -- process args
   local _, size, sigma, amplitude, normalize, 
   width, height, sigma_horz, sigma_vert = xlua.unpack(
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
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'}
   )
   
   -- local vars
   local center_x = width/2 + 0.5
   local center_y = height/2 + 0.5
   
   -- generate kernel
   local gauss = torch.Tensor(height, width)
   for i=1,height do
      for j=1,width do
         gauss[i][j] = amplitude * math.exp(-(math.pow((i-center_x)
                                                    /(sigma_horz*width),2)/2 
                                           + math.pow((j-center_y)
                                                   /(sigma_vert*height),2)/2))
      end
   end
   if normalize then
      gauss:div(gauss:sum())
   end
   return gauss
end

function image.gaussian1D(...)
   -- process args
   local _, size, sigma, amplitude, normalize
      = xlua.unpack(
      {...},
      'image.gaussian1D',
      'returns a 1D gaussian kernel',
      {arg='size', type='number', help='size the kernel', default=3},
      {arg='sigma', type='number', help='Sigma', default=0.25},
      {arg='amplitude', type='number', help='Amplitute of the gaussian (max value)', default=1},
      {arg='normalize', type='number', help='Normalize kernel (exc Amplitude)', default=false}
   )

   -- local vars
   local center = size/2 + 0.5
   
   -- generate kernel
   local gauss = torch.Tensor(size)
   for i=1,size do
      gauss[i] = amplitude * math.exp(-(math.pow((i-center)
                                              /(sigma*size),2)/2))
   end
   if normalize then
      gauss:div(gauss:sum())
   end
   return gauss
end

----------------------------------------------------------------------
--- Returns a Laplacian kernel.
-- The default parameters generate that occupies the entire kernel.
--
function image.laplacian(...)
   -- process args
   local _, size, sigma, amplitude, normalize, 
   width, height, sigma_horz, sigma_vert = xlua.unpack(
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
      {arg='sigma_vert', type='number', help='vertical sigma', defaulta='sigma'}
   )

   -- local vars
   local center_x = width/2 + 0.5
   local center_y = height/2 + 0.5
   
   -- generate kernel
   local logauss = torch.Tensor(height,width)
   for i=1,height do
      for j=1,width do
         local xsq = math.pow((i-center_x)/(sigma_horz*width),2)/2
         local ysq = math.pow((j-center_y)/(sigma_vert*height),2)/2
         local derivCoef = 1 - (xsq + ysq)
         logauss[i][j] = derivCoef * amplitude * math.exp(-(xsq + ysq))
      end
   end
   if normalize then
      logauss:div(logauss:sum())
   end
   return logauss
end
