
INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install image

TODO:
the package was imported from Torch5, and needs lots of
additions/tuning for Torch7...

 + rotate()           TODO     (fix C code: major->row mode)
 + scaleSimple()      TODO     (fix C code: major->row mode)
 + scaleBilinear()        DONE
 + translate()        TODO     (fix C code: major->row mode)
 + loadPNG()          TODO     (wrap libpng)
 + savePNG()          TODO     (wrap libpng)
 + loadJPG()              DONE
 + saveJPG()          TODO     (add call in libjpeg wrapper)
 + crop()             TODO     (fix C code: major->row mode)
 + convolve***()      TODO     (missing addT4dotT2)
 + display()          TODO     (missing qt.QImage.fromTensor)
 + rgb2yuv() and all  TODO     (import from xlearn package and major->row mode)
 + gaussian/laplacian TODO     (import from xlearn package)

USE:
$ lua
> require image
> l = image.lena()
> image.display(l)
