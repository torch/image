# image Package Reference Manual #
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
extension suffix. Supported formats are 
[JPEG](https://en.wikipedia.org/wiki/JPEG), 
[PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics), 
[PPM and PGM](https://en.wikipedia.org/wiki/Netpbm_format).
 
The returned `tensor` has size `nChannel x height x width` where `nChannel` is 
1 (greyscale) or 3 (usually [RGB](https://en.wikipedia.org/wiki/RGB_color_model) 
or [YUV](https://en.wikipedia.org/wiki/YUV).

## image.save(filename, tensor) ##
Saves Tensor `tensor` to disk at path `filename`. The format to which 
the image is saved is extrapolated from the `filename`'s extension suffix.
The `tensor` should be of size `nChannel x height x width`.

## [result] image.crop([dst,] src, startx, starty, [endx, endy]) ##
Crops image `src` at coordinate `(startx, starty)` up to coordinate 
`(endx, endy)`. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `result` Tensor.

## [result] image.translate([dst,] src, x, y) ##
Translates image `src` by `x` pixels horizontally and `y` pixels 
vertically. If `dst` is provided, it is used to store the output
image. Otherwise, returns a new `result` Tensor.

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

## [result] image.rotate([dst,], src, theta) ##
Rotates image `src` by `theta` radians. 
If `dst` is specified it is used to store the results of the rotation.

## [result] image.warp([dst,] src, field, [mode, offset, clamp]) ##
Warps image `src` (of size`KxHxW`) 
according to flow field `field`. The latter has size `2xHxW` where the 
first dimension is for the `(y,x)` flow field. String `mode` can 
take on values [lanczos](https://en.wikipedia.org/wiki/Lanczos_resampling), 
[bicubic](https://en.wikipedia.org/wiki/Bicubic_interpolation),
[bilinear](https://en.wikipedia.org/wiki/Bilinear_interpolation) (the default), 
or *simple*. When `offset` is true (the default), `(x,y)` is added to the flow field.
The `clamp` variable specifies how to handle the interpolation of samples off the input image.
Permitted values are strings *clamp* (the default) or *pad*. 
If `dst` is specified it is used to store the result of the warp.

## [result] image.hflip([dst,] src) ##
Flips image `src` horizontally (left<->right). If `dst` is provided, it is used to
store the output image. Otherwise, returns a new `result` Tensor.

## [result] image.vflip([dst,], src) ##
Flips image `src` vertically (upsize<->down). If `dst` is provided, it is used to
store the output image. Otherwise, returns a new `result` Tensor.

## [result] image.convolve([dst,] src, kernel, [mode]) ##
Convolves Tensor `kernel` over image `src`. Valid string values for argument 
`mode` are :
 * *full* : the `src` image is effectively zero-padded such that the `result` of the convolution has the same size as `src`;
 * *valid* (the default) : the `result` image will have `math.ceil(kernel/2)` less columns and rows on each side;
 * *same* : performs a *full* convolution, but crops out the portion fitting the output size of *valid*;
Note that this function internally uses 
[torch.conv2](https://github.com/torch/torch7/blob/master/doc/maths.md#torch.conv.dok).
If `dst` is provided, it is used to store the output image. 
Otherwise, returns a new `result` Tensor.

## [result] image.minmax{tensor,[min,max,symm,inplace,saturate,tensorOut]} ##
Compresses image `tensor` between `min` and `max`. 
When omitted, `min` and `max` are infered from 
`tensor:min()` and `tensor:max()`, respectively.
The `tensor` is normalized using `min` and `max` by performing :
```lua
tensor:add(-min):div(max-min)
```
When `symm=true` and `min` and `max` are both omitted, 
`max = min*2` in the above equation. The default is `false`.

When `inplace=true`, the result of the compression is stored in `tensor`. 
The default is `false`.

When `saturate=true`, the result of the compression is passed through
[image.saturate](#image.saturate)

When provided, Tensor `tensorOut` is used to store results.

Note that arguments should be provided as key-value pairs (in a table).

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
