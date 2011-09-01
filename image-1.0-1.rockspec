
package = "image"
version = "1.0-1"

source = {
   url = "image-1.0-1.tgz"
}

description = {
   summary = "An image processing toolbox, for Torch7",
   detailed = [[
         An image processing toolbox: to load/save images,
         rescale/rotate, remap colorspaces, ...
   ]],
   homepage = "",
   license = "MIT/X11" -- or whatever you like
}

dependencies = {
   "lua >= 5.1",
   "torch",
   "xlua"
}

build = {
   type = "cmake",
   variables = {
      CMAKE_INSTALL_PREFIX = "$(PREFIX)"
   }
}
