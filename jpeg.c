
#include <TH.h>
#include <luaT.h>
#include <jpeglib.h>
#include <setjmp.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_string_(NAME) TH_CONCAT_STRING_3(torch., Real, NAME)
#define libjpeg_(NAME) TH_CONCAT_3(libjpeg_, Real, NAME)

static const void* torch_FloatTensor_id = NULL;
static const void* torch_DoubleTensor_id = NULL;

#include "generic/jpeg.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libjpeg(lua_State *L)
{
  torch_FloatTensor_id = luaT_checktypename2id(L, "torch.FloatTensor");
  torch_DoubleTensor_id = luaT_checktypename2id(L, "torch.DoubleTensor");

  libjpeg_FloatMain_init(L);
  libjpeg_DoubleMain_init(L);

  luaL_register(L, "libjpeg.double", libjpeg_DoubleMain__);
  luaL_register(L, "libjpeg.float", libjpeg_FloatMain__);

  return 1;
}
