
#include <TH.h>
#include <luaT.h>

#if LUA_VERSION_NUM >= 503
#define luaL_checklong(L,n)     ((long)luaL_checkinteger(L, (n)))
#endif


#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define image_(NAME) TH_CONCAT_3(image_, Real, NAME)

#ifdef max
#undef max
#endif
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

#ifdef min
#undef min
#endif
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

#include "font.c"

#include "generic/image.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libimage(lua_State *L)
{
  image_FloatMain_init(L);
  image_DoubleMain_init(L);
  image_ByteMain_init(L);

  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setglobal(L, "image");

  lua_newtable(L);
  luaT_setfuncs(L, image_DoubleMain__, 0);
  lua_setfield(L, -2, "double");

  lua_newtable(L);
  luaT_setfuncs(L, image_FloatMain__, 0);
  lua_setfield(L, -2, "float");

  lua_newtable(L);
  luaT_setfuncs(L, image_ByteMain__, 0);
  lua_setfield(L, -2, "byte");

  return 1;
}
