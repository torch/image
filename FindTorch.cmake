
# Find Torch (luaT+TH)

if (TORCH_PREFIX)
   find_program (TORCH_EXECUTABLE lua ${TORCH_PREFIX}/bin NO_DEFAULT_PATH)
endif (TORCH_PREFIX)

if (NOT TORCH_EXECUTABLE)
   find_program (TORCH_EXECUTABLE lua PATH)
endif (NOT TORCH_EXECUTABLE)

if (TORCH_EXECUTABLE)
  get_filename_component (TORCH_BIN_DIR ${TORCH_EXECUTABLE} PATH)
endif (TORCH_EXECUTABLE)

find_library (TORCH_TH TH ${TORCH_BIN_DIR}/../lib)
find_library (TORCH_luaT luaT ${TORCH_BIN_DIR}/../lib)
find_library (TORCH_lua lua ${TORCH_BIN_DIR}/../lib)

set (TORCH_LIBRARIES ${TORCH_TH} ${TORCH_luaT} ${TORCH_lua})

find_path (TORCH_INCLUDE_DIR lua.h
           ${TORCH_BIN_DIR}/../include/ 
           NO_DEFAULT_PATH)

set (TORCH_INCLUDE_DIR ${TORCH_INCLUDE_DIR} ${TORCH_INCLUDE_DIR}/TH)

set (TORCH_PACKAGE_PATH "${TORCH_BIN_DIR}/../share/lua/5.1" CACHE PATH "where Lua searches for Lua packages")
set (TORCH_PACKAGE_CPATH "${TORCH_BIN_DIR}/../lib/lua/5.1" CACHE PATH "where Lua searches for library packages")

set (TORCH_PREFIX ${TORCH_BIN_DIR}/..)

mark_as_advanced (
  TORCH_PREFIX
  TORCH_EXECUTABLE
  TORCH_LIBRARIES
  TORCH_INCLUDE_DIR
  TORCH_PACKAGE_PATH
  TORCH_PACKAGE_CPATH
)

set (TORCH_FOUND 1)
if (NOT TORCH_TH)
   set (TORCH_FOUND 0)
endif (NOT TORCH_TH)
if (NOT TORCH_lua)
   set (TORCH_FOUND 0)
endif (NOT TORCH_lua)
if (NOT TORCH_luaT)
   set (TORCH_FOUND 0)
endif (NOT TORCH_luaT)
if (NOT TORCH_EXECUTABLE)
   set (TORCH_FOUND 0)
endif (NOT TORCH_EXECUTABLE)
if (NOT TORCH_INCLUDE_DIR)
   set (TORCH_FOUND 0)
endif (NOT TORCH_INCLUDE_DIR)

if (NOT TORCH_FOUND AND Lua_FIND_REQUIRED)
   message (FATAL_ERROR "Could not find Torch/Lua -- please install it!")
elseif (NOT TORCH_FOUND AND Lua_FIND_REQUIRED)
   message (STATUS "Lua bin found in " ${TORCH_BIN_DIR})
endif (NOT TORCH_FOUND AND Lua_FIND_REQUIRED)

if (NOT Lua_FIND_QUIETLY)
  if (TORCH_FOUND)
    message (STATUS "Lua bin found in " ${TORCH_BIN_DIR})
  else (TORCH_FOUND)
    message (STATUS "Lua bin not found. Please specify location")
  endif (TORCH_FOUND)
endif (NOT Lua_FIND_QUIETLY)
