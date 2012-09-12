#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/ppm.c"
#else

static int libppm_(Main_load)(lua_State *L)
{
  // get args
  const char *filename = luaL_checkstring(L, 1);

  // load file
  FILE* fp = fopen ( filename, "r" );
  if ( !fp ) {
    printf ( "Failed to open file '%s'!\n", filename );
  }

  // parse header
  char hdr[256]={};
  long W,H,C;
  int l=0;
  char p,n;
  int D;
  while ( sscanf ( hdr, "%c%c %ld %ld %d", &p, &n, &W, &H, &D ) < 5 ) {
    fgets ( hdr+l, 256-l, fp );
    char * comment = strchr ( hdr, 'p' );
    if ( comment ) l = hdr - comment;
    else l = strlen ( hdr );
    if ( l>=255 ) {
      W=H=0;
      fclose ( fp );
      luaL_error(L, "corrupted file");
    }
  }
  if ( p != 'P' ) {
    W=H=0;
    fclose ( fp );
    luaL_error(L, "corrupted file");
  }

  // load data
  unsigned char *r = NULL;
  if ( n=='6' ) {
    C = 3;
    r = (unsigned char *)malloc(W*H*C);
    fread ( r, 1, W*H*C, fp );
  } else if ( n=='5' ) {
    C = 1;
    r = (unsigned char *)malloc(W*H*C);
    fread ( r, 1, W*H*C, fp );
  } else if ( n=='3' ) {
    int c,i;
    C = 3;
    r = (unsigned char *)malloc(W*H*C);
    for (i=0; i<W*H*3; i++) {
      fscanf ( fp, "%d", &c );
      r[i] = 255*c / D;
    }
  } else if ( n=='2' ) {
    int c,i;
    C = 1;
    r = (unsigned char *)malloc(W*H*C);
    for (i=0; i<W*H*3; i++) {
      fscanf ( fp, "%d", &c );
      r[i] = 255*c / D;
    }
  } else {
    W=H=C=0;
    fclose ( fp );
    luaL_error(L, "corrupted file");
  }

  // export tensor
  THTensor *tensor = THTensor_(newWithSize3d)(C,H,W);
  real *data = THTensor_(data)(tensor);
  long i,k,j=0;
  for (i=0; i<W*H; i++) {
    for (k=0; k<C; k++) {
      data[k*H*W+i] = (real)r[j++];
    }
  }

  // cleanup
  free(r);
  fclose(fp);

  // return loaded image
  luaT_pushudata(L, tensor, torch_Tensor);
  return 1;
}

int libppm_(Main_save)(lua_State *L) {
  // get args
  const char *filename = luaL_checkstring(L, 1);
  THTensor *tensor = luaT_checkudata(L, 2, torch_Tensor);  
  THTensor *tensorc = THTensor_(newContiguous)(tensor);
  real *data = THTensor_(data)(tensorc);

  // dimensions
  long C,H,W,N;
  if (tensorc->nDimension == 3) {
    C = tensorc->size[0];
    H = tensorc->size[1];
    W = tensorc->size[2];
  } else if (tensorc->nDimension == 2) {
    C = 1;
    H = tensorc->size[0];
    W = tensorc->size[1];
  } else {
    C=W=H=0;
    luaL_error(L, "can only export tensor with geometry: HxW or 1xHxW or 3xHxW");
  }
  N = C*H*W;

  // convert to chars
  unsigned char *bytes = (unsigned char*)malloc(N);
  long i,k,j=0;
  for (i=0; i<W*H; i++) {
    for (k=0; k<C; k++) {
      bytes[j++] = (unsigned char)data[k*H*W+i];
    }
  }

  // open file
  FILE* fp = fopen(filename, "w");
  if ( !fp ) {
    luaL_error(L, "cannot open file <%s> for reading", filename);
  }

  // write 3 or 1 channel(s) header
  if (C == 3) {
    fprintf(fp, "P6\n%ld %ld\n%d\n", W, H, 255);
  } else {
    fprintf(fp, "P5\n%ld %ld\n%d\n", W, H, 255);
  }

  // write data
  fwrite(bytes, 1, N, fp);

  // cleanup
  THTensor_(free)(tensorc);
  free(bytes);
  fclose (fp);

  // return result
  return 1;
}

static const luaL_reg libppm_(Main__)[] =
{
  {"load", libppm_(Main_load)},
  {"save", libppm_(Main_save)},
  {NULL, NULL}
};

DLL_EXPORT int libppm_(Main_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, libppm_(Main__), "libppm");
  return 1;
}

#endif
