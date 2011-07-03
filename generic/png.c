#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/png.c"
#else

/*
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 * Clement: modified for Torch7.
 */

static THTensor * libpng_(read_png_file)(const char *file_name)
{
  char header[8];    // 8 is the maximum size that can be checked

  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep * row_pointers;

  /* open file and test for it being a png */
  FILE *fp = fopen(file_name, "rb");
  if (!fp)
    abort_("[read_png_file] File %s could not be opened for reading", file_name);
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
    abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    abort_("[read_png_file] png_create_read_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    abort_("[read_png_file] png_create_info_struct failed");

  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during init_io");

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  /* get depth */
  int depth;
  if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA)
    depth = 4;
  else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
    depth = 3;
  else if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
    depth = 1;
  else
    abort_("[read_png_file] Unknown color space");

  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[read_png_file] Error during read_image");

  /* alloc tensor */
  THTensor *tensor = THTensor_(newWithSize3d)(depth, height, width);
  real *tensor_data = THTensor_(data)(tensor);

  /* alloc data in lib format */
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  int y;
  for (y=0; y<height; y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  /* read image in */
  png_read_image(png_ptr, row_pointers);

  /* convert image to dest tensor */
  int x,k;
  for (k=0; k<depth; k++) {
    for (y=0; y<height; y++) {
      png_byte* row = row_pointers[y];
      for (x=0; x<width; x++) {
        //*tensor_data++ = (real)row[x*4+k];
        png_byte val = row[x*depth+k];
        THTensor_(set3d)(tensor, k, y, x, (real)val);
      }
    }
  }

  /* cleanup heap allocation */
  for (y=0; y<height; y++)
    free(row_pointers[y]);
  free(row_pointers);

  /* done with file */
  fclose(fp);

  /* return tensor */
  return tensor;
}

static void libpng_(write_png_file)(const char *file_name, THTensor *tensor)
{
  int width, height;
  png_byte color_type;
  png_byte bit_depth = 8;

  png_structp png_ptr;
  png_infop info_ptr;
  int number_of_passes;
  png_bytep * row_pointers;

  /* get dims and contiguous tensor */
  THTensor *tensorc = THTensor_(newContiguous)(tensor,0);
  real *tensor_data = THTensor_(data)(tensorc);
  long depth=0;
  if (tensorc->nDimension == 3) {
    depth = tensorc->size[0];
    height = tensorc->size[1];
    width = tensorc->size[2];
  } else if (tensorc->nDimension == 2) {
    depth = 1;
    height = tensorc->size[0];
    width = tensorc->size[1];    
  }

  /* depth check */
  if ((depth != 1) && (depth != 3) && (depth != 4)) {
    abort_("[write_png_file] Depth must be 1, 3 or 4");
  }
  if (depth == 4) color_type = PNG_COLOR_TYPE_RGBA;
  else if (depth == 3) color_type = PNG_COLOR_TYPE_RGB;
  else if (depth == 1) color_type = PNG_COLOR_TYPE_GRAY;

  /* create file */
  FILE *fp = fopen(file_name, "wb");
  if (!fp)
    abort_("[write_png_file] File %s could not be opened for writing", file_name);

  /* initialize stuff */
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
    abort_("[write_png_file] png_create_write_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    abort_("[write_png_file] png_create_info_struct failed");

  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during init_io");

  png_init_io(png_ptr, fp);

  /* write header */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during writing header");

  png_set_IHDR(png_ptr, info_ptr, width, height,
         bit_depth, color_type, PNG_INTERLACE_NONE,
         PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  /* convert tensor to 8bit bytes */
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  int y;
  for (y=0; y<height; y++)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  /* convert image to dest tensor */
  int x,k;
  for (k=0; k<depth; k++) {
    for (y=0; y<height; y++) {
      png_byte* row = row_pointers[y];
      for (x=0; x<width; x++) {
        row[x*depth+k] = (png_byte)THTensor_(get3d)(tensor, k, y, x);
        //row[x*4+k] = *tensor_data++;
      }
    }
  }

  /* write bytes */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during writing bytes");

  png_write_image(png_ptr, row_pointers);

  /* end write */
  if (setjmp(png_jmpbuf(png_ptr)))
    abort_("[write_png_file] Error during end of write");

  png_write_end(png_ptr, NULL);

  /* cleanup heap allocation */
  for (y=0; y<height; y++)
    free(row_pointers[y]);
  free(row_pointers);

  /* cleanup */
  fclose(fp);
  THTensor_(free)(tensorc);
}

static int libpng_(Main_load)(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  THTensor *tensor = libpng_(read_png_file)(filename);
  luaT_pushudata(L, tensor, torch_(Tensor_id));
  return 1;
}

static int libpng_(Main_save)(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  THTensor *tensor = luaT_checkudata(L, 2, torch_(Tensor_id));
  libpng_(write_png_file)(filename, tensor);
  return 0;
}

static const luaL_reg libpng_(Main__)[] =
{
  {"load", libpng_(Main_load)},
  {"save", libpng_(Main_save)},
  {NULL, NULL}
};

DLL_EXPORT int libpng_(Main_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, libpng_(Main__), "libpng");
  return 1;
}

#endif
