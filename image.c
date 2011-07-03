
#include <TH.h>
#include <luaT.h>

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_string_(NAME) TH_CONCAT_STRING_3(torch., Real, NAME)
#define image_(NAME) TH_CONCAT_3(image_, Real, NAME)

static const void* torch_FloatTensor_id = NULL;
static const void* torch_DoubleTensor_id = NULL;

#ifdef max
#undef max
#endif
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

#ifdef min
#undef min
#endif
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

#include "generic/image.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libimage(lua_State *L)
{
  torch_FloatTensor_id = luaT_checktypename2id(L, "torch.FloatTensor");
  torch_DoubleTensor_id = luaT_checktypename2id(L, "torch.DoubleTensor");

  image_FloatMain_init(L);
  image_DoubleMain_init(L);

  luaL_register(L, "image.double", image_DoubleMain__); 
  luaL_register(L, "image.float", image_FloatMain__);

  return 1;
}
