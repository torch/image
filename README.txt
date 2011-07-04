
INSTALL:
$ luarocks --from=http://data.neuflow.org/lua/rocks install image

TODO:
the package was imported from Torch5, and needs lots of
additions/tuning for Torch7...

 + scaleBilinear()    DONE
 + scaleSimple()      DONE
 + gaussian()         DONE
 + gaussian1D()       DONE
 + laplacian()        DONE
 + loadJPG()          DONE
 + saveJPG()          DONE
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
 + translate()        DONE
 + rotate()           DONE
 + crop()             DONE

in order of priority:
 + convolve***()      TODO     (missing addT4dotT2, waiting for lab.conv())

USE:
$ lua
> require image
> l = image.lena()
> image.display(l)
