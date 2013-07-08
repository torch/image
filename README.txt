DEPENDENCIES:
Torch7 (www.torch.ch)

INSTALL:
$ torch-rocks install image

USE:
$ torch
> require 'image'
> l = image.lena()
> image.display(l)
