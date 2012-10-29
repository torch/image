
#include <TH.h>
#include <luaT.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define libppm_(NAME) TH_CONCAT_3(libppm_, Real, NAME)

#include "generic/ppm.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libppm(lua_State *L)
{
  libppm_FloatMain_init(L);
  libppm_DoubleMain_init(L);
  libppm_ByteMain_init(L);

  luaL_register(L, "libppm.double", libppm_DoubleMain__);
  luaL_register(L, "libppm.float", libppm_FloatMain__);
  luaL_register(L, "libppm.byte", libppm_ByteMain__);

  return 1;
}
