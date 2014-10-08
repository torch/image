# Image Package Reference Manual #

 * saving and loading images
 
## [tensor] image.load(filename, depth, tensortype) ##
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
