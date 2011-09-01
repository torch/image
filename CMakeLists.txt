cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

IF(${CMAKE_PROJECT_NAME} STREQUAL "Torch")
  SET(TORCH_LIBRARIES TH luaT)
  SET(TORCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
ELSE(${CMAKE_PROJECT_NAME} STREQUAL "Torch")
  # infer path for Torch7
  string (REGEX REPLACE "(.*)lib/luarocks/rocks.*" "\\1" TORCH_PREFIX "${CMAKE_INSTALL_PREFIX}" )
  message (STATUS "Found Torch7, installed in: " ${TORCH_PREFIX})
  
  find_package (Torch REQUIRED)
ENDIF(${CMAKE_PROJECT_NAME} STREQUAL "Torch")

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
