
#include <TH.h>
#include <luaT.h>
#include <jpeglib.h>
#include <setjmp.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define libjpeg_(NAME) TH_CONCAT_3(libjpeg_, Real, NAME)

#include "generic/jpeg.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libjpeg(lua_State *L)
{
  libjpeg_FloatMain_init(L);
  libjpeg_DoubleMain_init(L);
  libjpeg_ByteMain_init(L);

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setglobal(L, "libjpeg");

  lua_newtable(L);
  luaT_setfuncs(L, libjpeg_DoubleMain__, 0);
  lua_setfield(L, -2, "double");

  lua_newtable(L);
  luaT_setfuncs(L, libjpeg_FloatMain__, 0);
  lua_setfield(L, -2, "float");

  lua_newtable(L);
  luaT_setfuncs(L, libjpeg_ByteMain__, 0);
  lua_setfield(L, -2, "byte");

  return 1;
}
