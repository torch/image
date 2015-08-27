<a name="image.saveload"></a>
## Saving and Loading ##
This sections includes functions for saving and loading different types 
of images to and from disk.

<a name="image.load"></a>
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

<a name="image.save"></a>
### image.save(filename, tensor) ###
Saves Tensor `tensor` to disk at path `filename`. The format to which 
the image is saved is extrapolated from the `filename`'s extension suffix.
The `tensor` should be of size `nChannel x height x width`.

<a name="image.decompressJPG"></a>
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

<a name="image.compressJPG"></a>
### [res] image.compressJPG(tensor, [quality]) ###
Compresses an image to a ByteTensor in memory.  Optional quality is between 1 and 100 and adjusts compression quality.
