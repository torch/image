# image Package Reference Manual #
__image__ is the [Torch7 distribution](http://torch.ch/) package for processing 
images. It contains a wide variety of functions divided into the following categories:
 * [Saving and loading](#image.saveload) images as JPEG, PNG, PPM and PGM;
 * [Simple transformations](#image.simpletrans) like translation, scaling and rotation;
 * [Parameterized transformations](#image.paramtrans) like convolutions and warping;
 * [Graphical user interfaces](#image.grapicalinter) like display and window;
 * [Color Space Conversions](#image.colorspace) from and to RGB, YUV, Lab, and HSL;
 * [Tensor Constructors](#image.tensorconst) for creating Lenna, Fabio and Gaussian and Laplacian kernels;

Note that unless speficied otherwise, this package deals with images of size 
`nChannel x height x width`.

<a name="image.saveload"/>
## Saving and Loading ##
This sections includes functions for saving and loading different types 
of images to and from disk.

<a name="image.load"/>
### [res] image.load(filename, [depth, tensortype]) ###
Loads an image located at path `filename` having `depth` channels (1 or 3)
into a [Tensor](https://github.com/torch/torch7/blob/master/doc/tensor.md#tensor)
of type `tensortype` (*float*, *double* or *byte*). The last two arguments 
are optional.

The image format is determined from the `filename`'s 
extension suffix. Supported formats are 
[JPEG](https://en.wikipedia.org/wiki/JPEG), 
[PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics), 
[PPM and PGM](https://en.wikipedia.org/wiki/Netpbm_format).
 
The returned `res` Tensor has size `nChannel x height x width` where `nChannel` is 
1 (greyscale) or 3 (usually [RGB](https://en.wikipedia.org/wiki/RGB_color_model) 
or [YUV](https://en.wikipedia.org/wiki/YUV).

<a name="image.save"/>
### image.save(filename, tensor) ###
Saves Tensor `tensor` to disk at path `filename`. The format to which 
the image is saved is extrapolated from the `filename`'s extension suffix.
The `tensor` should be of size `nChannel x height x width`.

<a name="image.decompressJPG"/>
### [res] image.decompressJPG(tensor, [depth, tensortype]) ###
Decompresses an image from a ByteTensor in memory having `depth` channels (1 or 3)
into a [Tensor](https://github.com/torch/torch7/blob/master/doc/tensor.md#tensor)
of type `tensortype` (*float*, *double* or *byte*). The last two arguments
are optional.

Usage:
```lua
local fin = torch.DiskFile(imfile, 'r')
fin:binary()
fin:seekEnd()
local file_size_bytes = fin:position() - 1
fin:seek(1)
local img_binary = torch.ByteTensor(file_size_bytes)
fin:readByte(img_binary:storage())
fin:close()
-- Then when you're ready to decompress the ByteTensor:
im = image.decompressJPG(img_binary)
```

<a name="image.compressJPG"/>
### [res] image.compressJPG(tensor, [quality]) ###
Compresses an image to a ByteTensor in memory.  Optional quality is between 1 and 100 and adjusts compression quality.

<a name="image.simpletrans"/>
## Simple Transformations ##
This section includes simple but very common image transformations 
like cropping, translation, scaling and rotation. 

<a name="image.crop"/>
### [res] image.crop([dst,] src, x1, y1, [x2, y2]) ###
Crops image `src` at coordinate `(x1, y1)` up to coordinate 
`(x2, y2)`. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.translate"/>
### [res] image.translate([dst,] src, x, y) ###
Translates image `src` by `x` pixels horizontally and `y` pixels 
vertically. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.scale"/>
### [res] image.scale(src, width, height, [mode]) ###
Rescale the height and width of image `src` to have 
width `width` and height `height`.  Variable `mode` specifies 
type of interpolation to be used. Valid values include 
[bilinear](https://en.wikipedia.org/wiki/Bilinear_interpolation)
(the default), [bicubic](https://en.wikipedia.org/wiki/Bicubic_interpolation),
or *simple* interpolation. Returns a new `res` Tensor.

### [res] image.scale(src, size, [mode]) ###
Rescale the height and width of image `src`. 
Variable `size` is a number or a string specifying the 
size of the result image. When `size` is a number, it specifies the 
maximum height or width of the output. When it is a string like 
*WxH* or *MAX* or *^MIN*, it specifies the `height x width`, maximum, or minimum height or 
width of the output, respectively.

### [res] image.scale(dst, src, [mode]) ###
Rescale the height and width of image `src` to fit the dimensions of 
Tensor `dst`. 

<a name="image.rotate"/>
### [res] image.rotate([dst,], src, theta, [mode]) ###
Rotates image `src` by `theta` radians. 
If `dst` is specified it is used to store the results of the rotation.
Variable `mode` specifies type of interpolation to be used. Valid values include 
*simple* (the default) or *bilinear* interpolation.

<a name="image.polar"/>
### [res] image.polar([dst,], src, [interpolation], [mode]) ###
Converts image `src` to polar coordinates. In the polar image, angular information is in the vertical direction and radius information in the horizontal direction.
If `dst` is specified it is used to store the polar image. If `dst` is not specified, its size is automatically determined. Variable `interpolation` specifies type of interpolation to be used. Valid values include *simple* (the default) or *bilinear* interpolation. Variable `mode` determines whether the *full* image is converted to the polar space (implying empty regions in the polar image), or whether only the *valid* central part of the polar transform is returned (the default).

<a name="image.logpolar"/>
### [res] image.logpolar([dst,], src, [interpolation], [mode]) ###
Converts image `src` to log-polar coordinates. In the log-polar image, angular information is in the vertical direction and log-radius information in the horizontal direction.
If `dst` is specified it is used to store the polar image. If `dst` is not specified, its size is automatically determined. Variable `interpolation` specifies type of interpolation to be used. Valid values include *simple* (the default) or *bilinear* interpolation. Variable `mode` determines whether the *full* image is converted to the log-polar space (implying empty regions in the log-polar image), or whether only the *valid* central part of the log-polar transform is returned (the default). 

<a name="image.hflip"/>
### [res] image.hflip([dst,] src) ###
Flips image `src` horizontally (left<->right). If `dst` is provided, it is used to
store the output image. Otherwise, returns a new `res` Tensor.

<a name="image.vflip"/>
### [res] image.vflip([dst,], src) ###
Flips image `src` vertically (upsize<->down). If `dst` is provided, it is used to
store the output image. Otherwise, returns a new `res` Tensor.

<a name="image.flip"/>
### [res] image.flip([dst,] src, flip_dim) ###
Flips image `src` along the specified dimension. If `dst` is provided, it is used to
store the output image. Otherwise, returns a new `res` Tensor.

<a name="image.minmax"/>
### [res] image.minmax{tensor, [min, max, ...]} ###
Compresses image `tensor` between `min` and `max`. 
When omitted, `min` and `max` are infered from 
`tensor:min()` and `tensor:max()`, respectively.
The `tensor` is normalized using `min` and `max` by performing :
```lua
tensor:add(-min):div(max-min)
```
Other optional arguments (`...`) include `symm`, `inplace`, `saturate`, and `tensorOut`.
When `symm=true` and `min` and `max` are both omitted, 
`max = min*2` in the above equation. This results in a symmetric dynamic 
range that is particularly useful for drawing filters. The default is `false`.
When `inplace=true`, the result of the compression is stored in `tensor`. 
The default is `false`.
When `saturate=true`, the result of the compression is passed through
[image.saturate](#image.saturate)
When provided, Tensor `tensorOut` is used to store results. 
Note that arguments should be provided as key-value pairs (in a table).

<a name="image.gaussianpyramid"/>
### [res] image.gaussianpyramid([dst,] src, scales) ###
Constructs a [Gaussian pyramid](https://en.wikipedia.org/wiki/Gaussian_pyramid)
of scales `scales` from a 2D or 3D `src` image or size 
`[nChannel x] width x height`. Each Tensor at index `i` 
in the returned list of Tensors has size  `[nChannel x] width*scales[i] x height*scales[i]`.

If list `dst` is provided, with or without Tensors, it is used to store the output images. 
Otherwise, returns a new `res` list of Tensors.

Internally, this function makes use of functions [image.gaussian](#image.gaussian),
[image.scale](#image.scale) and [image.convolve](#image.convolve).

<a name="image.paramtrans"/>
## Parameterized transformations ##
This section includes functions for performing transformations on 
images requiring parameter Tensors like a warp `field` or a convolution
`kernel`.

<a name="image.warp"/>
### [res] image.warp([dst,]src,field,[mode,offset,clamp_mode,pad_val]) ###
Warps image `src` (of size`KxHxW`) 
according to flow field `field`. The latter has size `2xHxW` where the 
first dimension is for the `(y,x)` flow field. String `mode` can 
take on values [lanczos](https://en.wikipedia.org/wiki/Lanczos_resampling), 
[bicubic](https://en.wikipedia.org/wiki/Bicubic_interpolation),
[bilinear](https://en.wikipedia.org/wiki/Bilinear_interpolation) (the default), 
or *simple*. When `offset` is true (the default), `(x,y)` is added to the flow field.
The `clamp_mode` variable specifies how to handle the interpolation of samples off the input image.
Permitted values are strings *clamp* (the default) or *pad*.
When `clamp_mode` equals `pad`, the user can specify the padding value with `pad_val` (default = 0). Note: setting this value when `clamp_mode` equals `clamp` will result in an error.
If `dst` is specified, it is used to store the result of the warp.
Otherwise, returns a new `res` Tensor.

<a name="image.convolve"/>
### [res] image.convolve([dst,] src, kernel, [mode]) ###
Convolves Tensor `kernel` over image `src`. Valid string values for argument 
`mode` are :
 * *full* : the `src` image is effectively zero-padded such that the `res` of the convolution has the same size as `src`;
 * *valid* (the default) : the `res` image will have `math.ceil(kernel/2)` less columns and rows on each side;
 * *same* : performs a *full* convolution, but crops out the portion fitting the output size of *valid*;
Note that this function internally uses 
[torch.conv2](https://github.com/torch/torch7/blob/master/doc/maths.md#torch.conv.dok).
If `dst` is provided, it is used to store the output image. 
Otherwise, returns a new `res` Tensor.

<a name="image.lcn"/>
### [res] image.lcn(src, [kernel]) ###
Local contrast normalization (LCN) on a given `src` image using kernel `kernel`.
If `kernel` is not given, then a default `9x9` Gaussian is used 
(see [image.gaussian](#image.gaussian)).

To prevent border effects, the image is first global contrast normalized
(GCN) by substracting the global mean and dividing by the global 
standard deviation.

Then the image is locally contrast normalized using the following equation:
```lua
res = (src - lm(src)) / sqrt( lm(src) - lm(src*src) )
```
where `lm(x)` is the local mean of each pixel in the image (i.e. 
`image.convolve(x,kernel)`) and  `sqrt(x)` is the element-wise 
square root of `x`. In other words, LCN performs 
local substractive and divisive normalization. 

Note that this implementation is different than the LCN Layer defined on page 3 of 
[What is the Best Multi-Stage Architecture for Object Recognition?](http://yann.lecun.com/exdb/publis/pdf/jarrett-iccv-09.pdf).

<a name="image.erode"/>
### [res] image.erode(src, [kernel, pad]) ###
Performs a [morphological erosion](https://en.wikipedia.org/wiki/Erosion_(morphology)) 
on binary (zeros and ones) image `src` using odd 
dimensioned morphological binary kernel `kernel`. 
The default is a kernel consisting of ones of size `3x3`. Number 
`pad` is the value to assume outside the image boundary when performing 
the convolution. The default is 1.

<a name="image.dilate"/>
### [res] image.dilate(src, [kernel, pad]) ###
Performs a [morphological dilation](https://en.wikipedia.org/wiki/Dilation_(morphology)) 
on binary (zeros and ones) image `src` using odd 
dimensioned morphological binary kernel `kernel`. 
The default is a kernel consisting of ones of size `3x3`. Number 
`pad` is the value to assume outside the image boundary when performing 
the convolution. The default is 0.

<a name="image.grapicalinter"/>
## Graphical User Interfaces ##
The following functions, except for [image.toDisplayTensor](#image.toDisplayTensor), 
require package [qtlua](https://github.com/torch/qtlua) and can only be 
accessed via the `qlua` Lua interpreter (as opposed to the 
[th](https://github.com/torch/trepl) or luajit interpreter).

<a name="image.toDisplayTensor"/>
### [res] image.toDisplayTensor(input, [...]) ###
Optional arguments `[...]` expand to `padding`, `nrow`, `scaleeach`, `min`, `max`, `symmetric`, `saturate`.
Returns a single `res` Tensor that contains a grid of all in the images in `input`.
The latter can either be a table of image Tensors of size `height x width` (greyscale) or 
`nChannel x height x width` (color), 
or a single Tensor of size `batchSize x nChannel x height x width` or `nChannel x height x width` 
where `nChannel=[3,1]`, `batchSize x height x width` or `height x width`.

When `scaleeach=false` (the default), all detected images 
are compressed with successive calls to [image.minmax](#image.minmax):
```lua
image.minmax{tensor=input[i], min=min, max=max, symm=symmetric, saturate=saturate}
```
`padding` specifies the number of padding pixels between images. The default is 0.
`nrow` specifies the number of images per row. The default is 6.

Note that arguments can also be specified as key-value arguments (in a table).

<a name="image.display"/>
### [res] image.display(input, [...]) ###
Optional arguments `[...]` expand to `zoom`, `min`, `max`, `legend`, `win`, 
`x`, `y`, `scaleeach`, `gui`, `offscreen`, `padding`, `symm`, `nrow`.
Displays `input` image(s) with optional saturation and zooming. 
The `input`, which is either a Tensor of size `HxW`, `KxHxW` or `Kx3xHxW`, or list,
is first prepared for display by passing it through [image.toDisplayTensor](#image.toDisplayTensor):
```lua
input = image.toDisplayTensor{
   input=input, padding=padding, nrow=nrow, saturate=saturate, 
   scaleeach=scaleeach, min=min, max=max, symmetric=symm
}
```
The resulting `input` will be displayed using [qtlua](https://github.com/torch/qtlua).
The displayed image will be zoomed by a factor of `zoom`. The default is 1.
If `gui=true` (the default), the graphical user inteface (GUI) 
is an interactive window that provides the user with the ability to zoom in or out. 
This can be turned off for a faster display. `legend` is a legend to be displayed,
which has a default value of `image.display`. `win` is an optional qt window descriptor.
If `x` and `y` are given, they are used to offset the image. Both default to 0.
When `offscreen=true`, rendering (to generate images) is performed offscreen.

<a name="image.window"/>
### [window, painter] image.window([...]) ###
Creates a window context for images. 
Optional arguments `[...]` expand to `hook_resize`, `hook_mousepress`, `hook_mousedoublepress`.
These have a default value of `nil`, but may correspond to commensurate qt objects.

<a name="image.colorspace"/>
## Color Space Conversions ##
This section includes functions for performing conversions between 
different color spaces.

<a name="image.rgb2lab"/>
### [res] image.rgb2lab([dst,] src) ###
Converts a `src` RGB image to [Lab](https://en.wikipedia.org/wiki/Lab_color_space). 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.rgb2yuv"/>
### [res] image.rgb2yuv([dst,] src) ###
Converts a RGB image to YUV. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.yuv2rgb"/>
### [res] image.yuv2rgb([dst,] src) ###
Converts a YUV image to RGB. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.rgb2y"/>
### [res] image.rgb2y([dst,] src) ###
Converts a RGB image to Y (discard U and V). 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.rgb2hsl"/>
### [res] image.rgb2hsl([dst,] src) ###
Converts a RGB image to [HSL](https://en.wikipedia.org/wiki/HSL_and_HSV). 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.hsl2rgb"/>
### [res] image.hsl2rgb([dst,] src) ###
Converts a HSL image to RGB. 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.rgb2hsv"/>
### [res] image.rgb2hsv([dst,] src) ###
Converts a RGB image to [HSV](https://en.wikipedia.org/wiki/HSL_and_HSV). 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.hsv2rgb"/>
### [res] image.hsv2rgb([dst,] src) ###
Converts a HSV image to RGB. 
If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `res` Tensor.

<a name="image.rgb2nrgb"/>
### [res] image.rgb2nrgb([dst,] src) ###
Converts an RGB image to normalized-RGB. 

<a name="image.y2jet"/>
### [res] image.y2jet([dst,] src) ###
Converts a L-levels (1 to L) greyscale image into a L-levels jet heat-map.
If `dst` is provided, it is used to store the output image. Otherwise, returns a new `res` Tensor.

This is particulary helpful for understanding the magnitude of the values of a matrix, or easily spot peaks in scalar field (like probability densities over a 2D area).
For example, you can run it as

```lua
image.display{image=image.y2jet(torch.linspace(1,10,10)), zoom=50}
```

<a name="image.tensorconst"/>
## Tensor Constructors ##
The following functions construct Tensors like Gaussian or 
Laplacian kernels, or images like Lenna and Fabio.

<a name="image.lena"/>
### [res] image.lena() ###
Returns the classic `Lenna.jpg` image as a `3 x 512 x 512` Tensor.

<a name="image.fabio"/>
### [res] image.fabio() ###
Returns the `fabio.jpg` image as a `257 x 271` Tensor.

<a name="image.gaussian"/>
### [res] image.gaussian([size, sigma, amplitude, normalize, [...]]) ###
Returns a 2D [Gaussian](https://en.wikipedia.org/wiki/Gaussian_function) 
kernel of size `height x width`. When used as a Gaussian smoothing operator in a 2D 
convolution, this kernel is used to `blur` images and remove detail and noise 
(ref.: [Gaussian Smoothing](http://homepages.inf.ed.ac.uk/rbf/HIPR2/gsmooth.htm)).
Optional arguments `[...]` expand to 
`width`, `height`, `sigma_horz`, `sigma_vert`, `mean_horz`, `mean_vert` and `tensor`.

The default value of `height` and `width` is `size`, where the latter 
has a default value of 3. The amplitude of the Gaussian (its maximum value) 
is `amplitude`. The default is 1. 
When `normalize=true`, the kernel is normalized to have a sum of 1.
This overrides the `amplitude` argument. The default is `false`.
The default value of the horizontal and vertical standard deviation 
`sigma_horz` and `sigma_vert` of the Gaussian kernel is `sigma`, where 
the latter has a default value of 0.25. The default values for the 
corresponding means `mean_horz` and `mean_vert` are 0.5. Both the 
standard deviations and means are relative to kernels of unit width and height
where the top-left corner is the origin. In other works, a mean of 0.5 is 
the center of the kernel size, while a standard deviation of 0.25 is a quarter
of it. When `tensor` is provided (a 2D Tensor), the `height`, `width` and `size` are ignored.
It is used to store the returned gaussian kernel.

Note that arguments can also be specified as key-value arguments (in a table).

<a name="image.gaussian1D"/>
### [res] image.gaussian1D([size, sigma, amplitude, normalize, mean, tensor]) ###
Returns a 1D Gaussian kernel of size `size`, mean `mean` and standard 
deviation `sigma`. 
Respectively, these arguments have default values of 3, 0.25 and 0.5. 
The amplitude of the Gaussian (its maximum value) 
is `amplitude`. The default is 1. 
When `normalize=true`, the kernel is normalized to have a sum of 1.
This overrides the `amplitude` argument. The default is `false`. Both the 
standard deviation and mean are relative to a kernel of unit size. 
In other works, a mean of 0.5 is the center of the kernel size, 
while a standard deviation of 0.25 is a quarter of it. 
When `tensor` is provided (a 1D Tensor), the `size` is ignored.
It is used to store the returned gaussian kernel.

Note that arguments can also be specified as key-value arguments (in a table).

<a name="image.laplacian"/>
### [res] image.laplacian([size, sigma, amplitude, normalize, [...]]) ###
Returns a 2D [Laplacian](https://en.wikipedia.org/wiki/Blob_detection#The_Laplacian_of_Gaussian) 
kernel of size `height x width`. 
When used in a 2D convolution, the Laplacian of an image highlights 
regions of rapid intensity change and is therefore often used for edge detection 
(ref.: [Laplacian/Laplacian of Gaussian](http://homepages.inf.ed.ac.uk/rbf/HIPR2/log.htm)).
Optional arguments `[...]` expand to 
`width`, `height`, `sigma_horz`, `sigma_vert`, `mean_horz`, `mean_vert`.

The default value of `height` and `width` is `size`, where the latter 
has a default value of 3. The amplitude of the Laplacian (its maximum value) 
is `amplitude`. The default is 1. 
When `normalize=true`, the kernel is normalized to have a sum of 1.
This overrides the `amplitude` argument. The default is `false`.
The default value of the horizontal and vertical standard deviation 
`sigma_horz` and `sigma_vert` of the Laplacian kernel is `sigma`, where 
the latter has a default value of 0.25. The default values for the 
corresponding means `mean_horz` and `mean_vert` are 0.5. Both the 
standard deviations and means are relative to kernels of unit width and height
where the top-left corner is the origin. In other works, a mean of 0.5 is 
the center of the kernel size, while a standard deviation of 0.25 is a quarter
of it.

<a name="image.colormap"/>
### [res] image.colormap(nColor) ###
Creates an optimally-spaced RGB color mapping of `nColor` colors. 
Note that the mapping is obtained by generating the colors around 
the HSV wheel, varying the Hue component.
The returned `res` Tensor has size `nColor x 3`. 

<a name="image.jetColormap"/>
### [res] image.jetColormap(nColor) ###
Creates a jet (blue to red) RGB color mapping of `nColor` colors.
The returned `res` Tensor has size `nColor x 3`. 

## Dependencies:
[Torch7](www.torch.ch)

## Install:
```
$ luarocks install image
```

## Use:
```
> require 'image'
> l = image.lena()
> image.display(l)
> f = image.fabio()
> image.display(f)
```
