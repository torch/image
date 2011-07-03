
#include <TH.h>
#include <luaT.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

void abort_(const char * s, ...)
{
  va_list args;
  va_start(args, s);
  vfprintf(stderr, s, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_string_(NAME) TH_CONCAT_STRING_3(torch., Real, NAME)
#define libpng_(NAME) TH_CONCAT_3(libpng_, Real, NAME)

static const void* torch_FloatTensor_id = NULL;
static const void* torch_DoubleTensor_id = NULL;

#include "generic/png.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libpng(lua_State *L)
{
  torch_FloatTensor_id = luaT_checktypename2id(L, "torch.FloatTensor");
  torch_DoubleTensor_id = luaT_checktypename2id(L, "torch.DoubleTensor");

  libpng_FloatMain_init(L);
  libpng_DoubleMain_init(L);

  luaL_register(L, "libpng.double", libpng_DoubleMain__);
  luaL_register(L, "libpng.float", libpng_FloatMain__);

  return 1;
}
