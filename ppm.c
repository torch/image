
#include <TH.h>
#include <luaT.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_string_(NAME) TH_CONCAT_STRING_3(torch., Real, NAME)
#define libppm_(NAME) TH_CONCAT_3(libppm_, Real, NAME)

static const void* torch_FloatTensor_id = NULL;
static const void* torch_DoubleTensor_id = NULL;

#include "generic/ppm.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libppm(lua_State *L)
{
  torch_FloatTensor_id = luaT_checktypename2id(L, "torch.FloatTensor");
  torch_DoubleTensor_id = luaT_checktypename2id(L, "torch.DoubleTensor");

  libppm_FloatMain_init(L);
  libppm_DoubleMain_init(L);

  luaL_register(L, "libppm.double", libppm_DoubleMain__);
  luaL_register(L, "libppm.float", libppm_FloatMain__);

  return 1;
}
