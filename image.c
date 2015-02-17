
#include <TH.h>
#include <luaT.h>

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

/*
 * Convert an sRGB color channel to a linear sRGB color channel.
 */
static inline float gamma_expand_sRGB(float nonlinear)
{
  return (nonlinear <= 0.04045)
          ? (nonlinear / 12.92)
          : (pow((nonlinear+0.055)/1.055, 2.4));
}

/*
 * Convert a linear sRGB color channel to a sRGB color channel.
 */
static inline float gamma_compress_sRGB(float linear)
{
  return (linear <= 0.0031308)
          ? (12.92 * linear)
          : (1.055 * pow(linear, 1.0/2.4) - 0.055);
}

#include "generic/image.c"
#include "THGenerateAllTypes.h"

DLL_EXPORT int luaopen_libimage(lua_State *L)
{
  image_FloatMain_init(L);
  image_DoubleMain_init(L);
  image_ByteMain_init(L);

  luaL_register(L, "image.double", image_DoubleMain__); 
  luaL_register(L, "image.float", image_FloatMain__);
  luaL_register(L, "image.byte", image_ByteMain__);

  return 1;
}
