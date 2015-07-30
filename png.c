
#include <TH.h>
#include <luaT.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define libpng_(NAME) TH_CONCAT_3(libpng_, Real, NAME)

#include "generic/png.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libpng(lua_State *L)
{
  libpng_FloatMain_init(L);
  libpng_DoubleMain_init(L);
  libpng_ByteMain_init(L);

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setglobal(L, "libpng");

  lua_newtable(L);
  luaT_setfuncs(L, libpng_DoubleMain__, 0);
  lua_setfield(L, -2, "double");

  lua_newtable(L);
  luaT_setfuncs(L, libpng_FloatMain__, 0);
  lua_setfield(L, -2, "float");

  lua_newtable(L);
  luaT_setfuncs(L, libpng_ByteMain__, 0);
  lua_setfield(L, -2, "byte");

  return 1;
}
