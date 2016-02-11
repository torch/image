package = "image"
version = "1.1.alpha-0"

source = {
   url = "git://github.com/torch/image",
   tag = "master"
}

description = {
   summary = "An image library for Torch",
   detailed = [[
This package provides routines to load/save and manipulate images
using Torch's Tensor data structure.
   ]],
   homepage = "https://github.com/torch/image",
   license = "BSD"
}

dependencies = {
   "torch >= 7.0",
   "sys >= 1.0",
   "xlua >= 1.0",
   "dok"
}

build = {
   type = "command",
   build_command = [[
cmake -E make_directory build && cd build && cmake .. -DLUALIB=$(LUALIB) -DLUA_INCDIR="$(LUA_INCDIR)" -DLUA_LIBDIR="$(LUA_LIBDIR)"  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(LUA_BINDIR)/.." -DCMAKE_INSTALL_PREFIX="$(PREFIX)" && $(MAKE)
   ]],
   install_command = "cd build && $(MAKE) install"
}
