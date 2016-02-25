# image Package Reference Manual #

[![Build Status](https://travis-ci.org/torch/image.svg)](https://travis-ci.org/torch/image) 

__image__ is the [Torch7 distribution](http://torch.ch/) package for processing 
images. It contains a wide variety of functions divided into the following categories:

  * [Saving and loading](doc/saveload.md) images as JPEG, PNG, PPM and PGM;
  * [Simple transformations](doc/simpletransform.md) like translation, scaling and rotation;
  * [Parameterized transformations](doc/paramtransform.md) like convolutions and warping;
  * [Simple Drawing Routines](doc/drawing.md) like drawing text on an image;
  * [Graphical user interfaces](doc/gui.md) like display and window;
  * [Color Space Conversions](doc/colorspace.md) from and to RGB, YUV, Lab, and HSL;
  * [Tensor Constructors](doc/tensorconstruct.md) for creating Lenna, Fabio and Gaussian and Laplacian kernels;

Note that unless speficied otherwise, this package deals with images of size 
`nChannel x height x width`.

## Install

The easiest way to install this package it by following the [intructions](http://torch.ch/docs/getting-started.html) 
to install [Torch7](http://www.torch.ch), which includes __image__. 
Otherwise, to update or manually re-install it:

```bash
$ luarocks install image
```

You can test your install with:

```bash
$ luajit -limage -e "image.test()"
```

## Usage

```lua
> require 'image'
> l = image.lena()
> image.display(l)
> f = image.fabio()
> image.display(f)
```
