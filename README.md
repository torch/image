# Image Package Reference Manual #
Unless speficied otherwise, this package deals with images of size 
`nChannel x height x width`.
 * saving and loading images;
 * translation, scaling and warping;
 
## [tensor] image.load(filename, [depth, tensortype]) ##
Loads an image located at path `filename` having `depth` channels (1 or 3)
into a [Tensor](https://github.com/torch/torch7/blob/master/doc/tensor.md#tensor)
of type `tensortype` (*float*, *double* or *byte*). The last two arguments 
are optional.

The image format is determined from the `filename`'s 
extension suffix. Supported formats are:
 * [JPEG](https://en.wikipedia.org/wiki/JPEG)
 * [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics) 
 * [PPM and PGM](https://en.wikipedia.org/wiki/Netpbm_format)
 
The returned `tensor` has size `nChannel x height x width` where `nChannel` is 
1 (greyscale) or 3 (usually [RGB](https://en.wikipedia.org/wiki/RGB_color_model) 
or [YUV](https://en.wikipedia.org/wiki/YUV).

## image.save(filename, tensor) ##
Saves Tensor `tensor` to disk at path `filename`. The format to which 
the image is saved is extrapolated from the `filename`'s extension suffix.
The `tensor` should be of size `nChannel x height x width`.

## [result] image.translate([dst,] src, x, y) ##
Translates image `src` by `x` pixels horizontally and `y` pixels 
vertically. If `dst` is provided, it is used to to store the output
image. Otherwise, return a new `result` Tensor.

## [result] image.scale(src, width, height, [mode]) ##
Rescale the height and width of image `src` to have 
width `width` and height `height`.  Variable `mode` specifies 
type of interpolation to be used. Valid values include 
[bilinear](https://en.wikipedia.org/wiki/Bilinear_interpolation)
(the default) or *simple* interpolation. Returns a new `result` Tensor.

### [result] image.scale(src, size, [mode]) ###
Rescale the height and width of image `src`. 
Variable `size` is a number or a string specifying the 
size of the result image. When `size` is a number, it specifies the 
maximum height or width of the output. When it is a string like 
*WxH* or *MAX* or *^MIN*, it specifies the `height x width`, maximum, or minimum height or 
width of the output, respectively.

### [result] image.scale(dst, src, [mode]) ###
Rescale the height and width of image `src` to fit the dimensions of 
Tensor `dst`. 

### [result] image.warp([dst,] src, field, [mode, offsetMode, clampMode]) ###
Warps image `src` (of size`KxHxW`) 
according to flow field `field`. The latter has size `2xHxW` where the 
first dimension is for the `(y,x)` flow field. String `mode` can 
take on values [lanczos](https://en.wikipedia.org/wiki/Lanczos_resampling), 
[bicubic](https://en.wikipedia.org/wiki/Bicubic_interpolation),
[bilinear](https://en.wikipedia.org/wiki/Bilinear_interpolation) (the default), 
or *simple*. When `offsetMode` is true (the default), `(x,y)` is added to the flow field.
The `clampMode` variable specifies how to handle the interpolation of samples off the input image.
Permitted values are *clamp* (the default) and *pad*. If `dst` is specified it is used to store the results of the warp.

## Dependencies:
Torch7 (www.torch.ch)

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
