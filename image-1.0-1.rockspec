
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

   cmake = [[
         cmake_minimum_required(VERSION 2.8)

         set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

         find_package (Torch REQUIRED)
         find_package (JPEG QUIET)
         find_package (PNG QUIET)

         set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

         if (JPEG_FOUND)
             include_directories (${JPEG_INCLUDE_DIR} ${TORCH_INCLUDE_DIR} ${PROJECT_SOURCE_DIR})
             add_library (jpeg SHARED jpeg.c)
             target_link_libraries (jpeg ${TORCH_LIBRARIES} ${JPEG_LIBRARIES})
             install_targets (/lib jpeg)
         else (JPEG_FOUND)
             message ("WARNING: Could not find JPEG libraries, JPEG wrapper will not be installed")
         endif (JPEG_FOUND)

         if (PNG_FOUND)
             include_directories (${PNG_INCLUDE_DIR} ${TORCH_INCLUDE_DIR} ${PROJECT_SOURCE_DIR})
             add_library (png SHARED png.c)
             target_link_libraries (png ${TORCH_LIBRARIES} ${PNG_LIBRARIES})
             install_targets (/lib png)
         else (PNG_FOUND)
             message ("WARNING: Could not find PNG libraries, PNG wrapper will not be installed")
         endif (PNG_FOUND)

         include_directories (${TORCH_INCLUDE_DIR} ${PROJECT_SOURCE_DIR})
         add_library (image SHARED image.c)
         link_directories (${TORCH_LIBRARY_DIR})
         target_link_libraries (image ${TORCH_LIBRARIES})

         install_files(/lua/image init.lua)
         install_files(/lua/image lena.jpg)
         install_files(/lua/image lena.png)
         install_files(/lua/image win.ui)
         install_targets(/lib image)
   ]],

   variables = {
      CMAKE_INSTALL_PREFIX = "$(PREFIX)"
   }
}
