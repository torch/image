
INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install image

TODO:
the package was imported from Torch5, and needs lots of
additions/tuning for Torch7...

 + scaleBilinear()    DONE
 + gaussian()         DONE
 + gaussian1D()       DONE
 + laplacian()        DONE
 + loadJPG()          DONE
 + loadPNG()          DONE
 + savePNG()          DONE
 + rgb2yuv()          DONE
 + yuv2rgb()          DONE
 + rgb2y()            DONE
 + rgb2nrgb()         DONE
 + rgb2hsl()          DONE
 + rgb2hsv()          DONE
 + hsl2rgb()          DONE
 + hsv2rgb()          DONE
 + display()          DONE

in order of priority:
 + scaleSimple()      TODO     (fix C code: major->row mode)
 + rotate()           TODO     (fix C code: major->row mode)
 + translate()        TODO     (fix C code: major->row mode)
 + crop()             TODO     (fix C code: major->row mode)
 + convolve***()      TODO     (missing addT4dotT2)
 + saveJPG()          TODO     (add call in libjpeg wrapper)

USE:
$ lua
> require image
> l = image.lena()
> image.display(l)
