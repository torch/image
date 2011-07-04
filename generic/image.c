#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/image.c"
#else

#undef TAPI
#define TAPI __declspec(dllimport)

static void image_(Main_op_validate)( lua_State *L,  THTensor *Tsrc, THTensor *Tdst){

  long src_depth = 1;
  long dst_depth = 1;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

  if(Tdst->nDimension == 3) dst_depth =  Tdst->size[0];
  if(Tsrc->nDimension == 3) src_depth =  Tsrc->size[0];

  if( (Tdst->nDimension==3 && ( src_depth!=dst_depth)) ||
      (Tdst->nDimension!=Tsrc->nDimension) )
    luaL_error(L, "image.scale: src and dst depths do not match");

  if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.scale: src and dst depths do not match");
}

static long image_(Main_op_stride)( THTensor *T,int i){
  if (T->nDimension == 2) {
    if (i == 0) return 0;
    else return T->stride[i-1];
  }
  return T->stride[i];
}

static long image_(Main_op_depth)( THTensor *T){
  if(T->nDimension == 3) return T->size[0]; /* rgb or rgba */
  return 1; /* greyscale */
}

static void image_(Main_scale_rowcol)(THTensor *Tsrc,
                                      THTensor *Tdst,
                                      long src_start,
                                      long dst_start,
                                      long src_stride,
                                      long dst_stride,
                                      long src_len,
                                      long dst_len ) {

  real *src= THTensor_(data)(Tsrc);
  real *dst= THTensor_(data)(Tdst);

  if ( dst_len > src_len ){
    long di;
    real si_f;
    long si_i;
    real scale = (real)(src_len - 1) / (dst_len - 1);

    for( di = 0; di < dst_len - 1; di++ ) {
      long dst_pos = dst_start + di*dst_stride;
      si_f = di * scale; si_i = (long)si_f; si_f -= si_i;

      dst[dst_pos] = (1 - si_f) * src[ src_start + si_i * src_stride ] +
        si_f * src[ src_start + (si_i + 1) * src_stride ];
    }

    dst[ dst_start + (dst_len - 1) * dst_stride ] =
      src[ src_start + (src_len - 1) * src_stride ];
  }
  else if ( dst_len < src_len ) {
    long di;
    long si0_i = 0; real si0_f = 0;
    long si1_i; real si1_f;
    long si;
    real scale = (real)src_len / dst_len;
    real acc, n;
    for( di = 0; di < dst_len; di++ )
      {
        si1_f = (di + 1) * scale; si1_i = (long)si1_f; si1_f -= si1_i;
        acc = (1 - si0_f) * src[ src_start + si0_i * src_stride ];
        n = 1 - si0_f;
        for( si = si0_i + 1; si < si1_i; si++ )
          {
            acc += src[ src_start + si * src_stride ];
            n += 1;
          }
        if( si1_i < src_len )
          {
            acc += si1_f * src[ src_start + si1_i*src_stride ];
            n += si1_f;
          }
        dst[ dst_start + di*dst_stride ] = acc / n;
        si0_i = si1_i; si0_f = si1_f;
      }
  }
  else {
    long i;
    for( i = 0; i < dst_len; i++ )
      dst[ dst_start + i*dst_stride ] = src[ src_start + i*src_stride ];
  }
}

static int image_(Main_scaleBilinear)(lua_State *L) {

  THTensor *Tsrc = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *Tdst = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *Ttmp;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height;
  long src_stride0, src_stride1, src_stride2, src_width, src_height;
  long tmp_stride0, tmp_stride1, tmp_stride2, tmp_width, tmp_height;
  long i, j, k;

  image_(Main_op_validate)(L, Tsrc,Tdst);

  int ndims;
  if (Tdst->nDimension == 3) ndims = 3;
  else ndims = 2;

  Ttmp = THTensor_(newWithSize2d)(Tsrc->size[ndims-2], Tdst->size[ndims-1]);

  dst_stride0= image_(Main_op_stride)(Tdst,0);
  dst_stride1= image_(Main_op_stride)(Tdst,1);
  dst_stride2= image_(Main_op_stride)(Tdst,2);
  src_stride0= image_(Main_op_stride)(Tsrc,0);
  src_stride1= image_(Main_op_stride)(Tsrc,1);
  src_stride2= image_(Main_op_stride)(Tsrc,2);
  tmp_stride0= image_(Main_op_stride)(Ttmp,0);
  tmp_stride1= image_(Main_op_stride)(Ttmp,1);
  tmp_stride2= image_(Main_op_stride)(Ttmp,2);
  dst_width=   Tdst->size[ndims-1];
  dst_height=  Tdst->size[ndims-2];
  src_width=   Tsrc->size[ndims-1];
  src_height=  Tsrc->size[ndims-2];
  tmp_width=   Ttmp->size[1];
  tmp_height=  Ttmp->size[0];

  for(k=0;k<image_(Main_op_depth)(Tsrc);k++) {
    /* compress/expand rows first */
    for(j = 0; j < src_height; j++) {
      image_(Main_scale_rowcol)(Tsrc,
				Ttmp,
				0*src_stride2+j*src_stride1+k*src_stride0,
				0*tmp_stride2+j*tmp_stride1+k*tmp_stride0,
				src_stride2,
				tmp_stride2,
				src_width,
				tmp_width );

    }

    /* then columns */
    for(i = 0; i < dst_width; i++) {
      image_(Main_scale_rowcol)(Ttmp,
				Tdst,
				i*tmp_stride2+0*tmp_stride1+k*tmp_stride0,
				i*dst_stride2+0*dst_stride1+k*dst_stride0,
				tmp_stride1,
				dst_stride1,
				tmp_height,
				dst_height );
    }
  }
  THTensor_(free)(Ttmp);
  return 0;
}

static int image_(Main_scaleSimple)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *Tdst = luaT_checkudata(L, 2, torch_(Tensor_id));
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  real scx, scy;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "image.scale: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "image.scale: dst not 2 or 3 dimensional");

  src= THTensor_(data)(Tsrc);
  dst= THTensor_(data)(Tdst);

  dst_stride0 = 0;
  dst_stride1 = Tdst->stride[Tdst->nDimension-2];
  dst_stride2 = Tdst->stride[Tdst->nDimension-1];
  dst_depth =  0;
  dst_height = Tdst->size[Tdst->nDimension-2];
  dst_width = Tdst->size[Tdst->nDimension-1];
  if(Tdst->nDimension == 3) {
    dst_stride0 = Tdst->stride[0];
    dst_depth = Tdst->size[0];
  }

  src_stride0 = 0;
  src_stride1 = Tsrc->stride[Tsrc->nDimension-2];
  src_stride2 = Tsrc->stride[Tsrc->nDimension-1];
  src_depth =  0;
  src_height = Tsrc->size[Tsrc->nDimension-2];
  src_width = Tsrc->size[Tsrc->nDimension-1];
  if(Tsrc->nDimension == 3) {
    src_stride0 = Tsrc->stride[0];
    src_depth = Tsrc->size[0];
  }

  if( (Tdst->nDimension==3 && ( src_depth!=dst_depth)) ||
      (Tdst->nDimension!=Tsrc->nDimension) ) {
    printf("image.scale:%d,%d,%ld,%ld\n",Tsrc->nDimension,Tdst->nDimension,src_depth,dst_depth);
    luaL_error(L, "image.scale: src and dst depths do not match");
  }

  if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.scale: src and dst depths do not match");

  /* printf("%d,%d -> %d,%d\n",src_width,src_height,dst_width,dst_height); */
  scx=((real)src_width)/((real)dst_width);
  scy=((real)src_height)/((real)dst_height);

  for(j = 0; j < dst_height; j++) {
    for(i = 0; i < dst_width; i++) {
      real val = 0.0;
      long ii=(long) (0.5+((real)i)*scx);
      long jj=(long) (0.5+((real)j)*scy);
      if(ii>src_width-1) ii=src_width-1;
      if(jj>src_height-1) jj=src_height-1;

      if(Tsrc->nDimension==2)
        {
          val=src[ii*src_stride2+jj*src_stride1];
          dst[i*dst_stride2+j*dst_stride1] = val;
        }
      else
        {
          for(k=0;k<src_depth;k++)
            {
              val=src[ii*src_stride2+jj*src_stride1+k*src_stride0];
              dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
            }
        }
    }
  }
  return 0;
}

static int image_(Main_rotate)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *Tdst = luaT_checkudata(L, 2, torch_(Tensor_id));
  real theta = luaL_checknumber(L, 3);
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  real xc, yc;
  real id,jd;
  long ii,jj;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

  src= THTensor_(data)(Tsrc);
  dst= THTensor_(data)(Tdst);

  dst_stride0 = 0;
  dst_stride1 = Tdst->stride[Tdst->nDimension-2];
  dst_stride2 = Tdst->stride[Tdst->nDimension-1];
  dst_depth =  0;
  dst_height = Tdst->size[Tdst->nDimension-2];
  dst_width = Tdst->size[Tdst->nDimension-1];
  if(Tdst->nDimension == 3) {
    dst_stride0 = Tdst->stride[0];
    dst_depth = Tdst->size[0];
  }

  src_stride0 = 0;
  src_stride1 = Tsrc->stride[Tsrc->nDimension-2];
  src_stride2 = Tsrc->stride[Tsrc->nDimension-1];
  src_depth =  0;
  src_height = Tsrc->size[Tsrc->nDimension-2];
  src_width = Tsrc->size[Tsrc->nDimension-1];
  if(Tsrc->nDimension == 3) {
    src_stride0 = Tsrc->stride[0];
    src_depth = Tsrc->size[0];
  }

  if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.rotate: src and dst depths do not match");

  if( (Tsrc->nDimension!=Tdst->nDimension) )
    luaL_error(L, "image.rotate: src and dst depths do not match");

  xc=src_width/2.0;
  yc=src_height/2.0;

  for(j = 0; j < dst_height; j++) {
    jd=j;
    for(i = 0; i < dst_width; i++) {
      real val = -1;
      id= i;

      ii=(long)( cos(theta)*(id-xc)-sin(theta)*(jd-yc) );
      jj=(long)( cos(theta)*(jd-yc)+sin(theta)*(id-xc) );
      ii+=(long) xc; jj+=(long) yc;

      /* rotated corners are blank */
      if(ii>src_width-1) val=0;
      if(jj>src_height-1) val=0;
      if(ii<0) val=0;
      if(jj<0) val=0;

      if(Tsrc->nDimension==2)
        {
          if(val==-1)
            val=src[ii*src_stride2+jj*src_stride1];
          dst[i*dst_stride2+j*dst_stride1] = val;
        }
      else
        {
          int do_copy=0; if(val==-1) do_copy=1;
          for(k=0;k<src_depth;k++)
            {
              if(do_copy)
                val=src[ii*src_stride2+jj*src_stride1+k*src_stride0];
              dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
            }
        }
    }
  }
  return 0;
}

static int image_(Main_cropNoScale)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *Tdst = luaT_checkudata(L, 2, torch_(Tensor_id));
  long startx = luaL_checklong(L, 3);
  long starty = luaL_checklong(L, 4);
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

  src= THTensor_(data)(Tsrc);
  dst= THTensor_(data)(Tdst);

  dst_stride0 = 0;
  dst_stride1 = Tdst->stride[Tdst->nDimension-2];
  dst_stride2 = Tdst->stride[Tdst->nDimension-1];
  dst_depth =  0;
  dst_height = Tdst->size[Tdst->nDimension-2];
  dst_width = Tdst->size[Tdst->nDimension-1];
  if(Tdst->nDimension == 3) {
    dst_stride0 = Tdst->stride[0];
    dst_depth = Tdst->size[0];
  }

  src_stride0 = 0;
  src_stride1 = Tsrc->stride[Tsrc->nDimension-2];
  src_stride2 = Tsrc->stride[Tsrc->nDimension-1];
  src_depth =  0;
  src_height = Tsrc->size[Tsrc->nDimension-2];
  src_width = Tsrc->size[Tsrc->nDimension-1];
  if(Tsrc->nDimension == 3) {
    src_stride0 = Tsrc->stride[0];
    src_depth = Tsrc->size[0];
  }

  if( startx<0 || starty<0 || (startx+dst_width>src_width) || (starty+dst_height>src_height))
    luaL_error(L, "image.crop: crop goes outside bounds of src");

  if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.crop: src and dst depths do not match");

  for(j = 0; j < dst_height; j++) {
    for(i = 0; i < dst_width; i++) {
      real val = 0.0;

      long ii=i+startx;
      long jj=j+starty;

      if(Tsrc->nDimension==2)
        {
          val=src[ii*src_stride2+jj*src_stride1];
          dst[i*dst_stride2+j*dst_stride1] = val;
        }
      else
        {
          for(k=0;k<src_depth;k++)
            {
              val=src[ii*src_stride2+jj*src_stride1+k*src_stride0];
              dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
            }
        }
    }
  }
  return 0;
}

static int image_(Main_translate)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *Tdst = luaT_checkudata(L, 2, torch_(Tensor_id));
  long shiftx = luaL_checklong(L, 3);
  long shifty = luaL_checklong(L, 4);
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;

  luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "rotate: src not 2 or 3 dimensional");
  luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "rotate: dst not 2 or 3 dimensional");

  src= THTensor_(data)(Tsrc);
  dst= THTensor_(data)(Tdst);

  dst_stride0 = 0;
  dst_stride1 = Tdst->stride[Tdst->nDimension-2];
  dst_stride2 = Tdst->stride[Tdst->nDimension-1];
  dst_depth =  0;
  dst_height = Tdst->size[Tdst->nDimension-2];
  dst_width = Tdst->size[Tdst->nDimension-1];
  if(Tdst->nDimension == 3) {
    dst_stride0 = Tdst->stride[0];
    dst_depth = Tdst->size[0];
  }

  src_stride0 = 0;
  src_stride1 = Tsrc->stride[Tsrc->nDimension-2];
  src_stride2 = Tsrc->stride[Tsrc->nDimension-1];
  src_depth =  0;
  src_height = Tsrc->size[Tsrc->nDimension-2];
  src_width = Tsrc->size[Tsrc->nDimension-1];
  if(Tsrc->nDimension == 3) {
    src_stride0 = Tsrc->stride[0];
    src_depth = Tsrc->size[0];
  }

  if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.crop: src and dst depths do not match");

  for(j = 0; j < src_height; j++) {

    for(i = 0; i < src_width; i++) {
      real val = 0.0;

      long ii=i+shiftx;
      long jj=j+shifty;

      if(ii<dst_width && jj<dst_height)
        /* check it's within destination bounds, else crop */
        {
          if(Tsrc->nDimension==2)
            {
              val=src[i*src_stride2+j*src_stride1];
              dst[ii*dst_stride2+jj*dst_stride1] = val;
            }
          else
            {
              for(k=0;k<src_depth;k++)
                {
                  val=src[i*src_stride2+j*src_stride1+k*src_stride0];
                  dst[ii*dst_stride2+jj*dst_stride1+k*dst_stride0] = val;
                }
            }
        }
    }
  }
  return 0;
}

static int image_(Main_saturate)(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *output = input;

  TH_TENSOR_APPLY2(real, output, real, input,                       \
                   *output_data = (*input_data < 0) ? 0 : (*input_data > 1) ? 1 : *input_data;)

  return 1;
}

/*
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 1] and
 * returns h, s, and l in the set [0, 1].
 */
int image_(Main_rgb2hsl)(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *hsl = luaT_checkudata(L, 2, torch_(Tensor_id));

  int y,x;
  real r,g,b,h,s,l;
  for (y=0; y<rgb->size[1]; y++) {
    for (x=0; x<rgb->size[2]; x++) {
      // get Rgb
      r = THTensor_(get3d)(rgb, 0, y, x);
      g = THTensor_(get3d)(rgb, 1, y, x);
      b = THTensor_(get3d)(rgb, 2, y, x);

      real mx = max(max(r, g), b);
      real mn = min(min(r, g), b);
      h = (mx + mn) / 2;
      s = h;
      l = h;

      if(mx == mn) {
        h = 0; // achromatic
        s = 0;
      } else {
        real d = mx - mn;
        s = l > 0.5 ? d / (2 - mx - mn) : d / (mx + mn);
        if (mx == r) {
          h = (g - b) / d + (g < b ? 6 : 0);
        } else if (mx == g) {
          h = (b - r) / d + 2; 
        } else {
          h = (r - g) / d + 4;
        }
        h /= 6;
      }

      // set hsl
      THTensor_(set3d)(hsl, 0, y, x, h);
      THTensor_(set3d)(hsl, 1, y, x, s);
      THTensor_(set3d)(hsl, 2, y, x, l);
    }
  }
  return 0;
}

// helper
static inline real image_(hue2rgb)(real p, real q, real t) {
  if (t < 0.) t += 1;
  if (t > 1.) t -= 1;
  if (t < 1./6) 
    return p + (q - p) * 6. * t;
  else if (t < 1./2) 
    return q;
  else if (t < 2./3) 
    return p + (q - p) * (2./3 - t) * 6.;
  else
    return p;                                       
}

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 1].
 */
int image_(Main_hsl2rgb)(lua_State *L) {
  THTensor *hsl = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *rgb = luaT_checkudata(L, 2, torch_(Tensor_id));

  int y,x;
  real r,g,b,h,s,l;
  for (y=0; y<hsl->size[1]; y++) {
    for (x=0; x<hsl->size[2]; x++) {
      // get hsl
      h = THTensor_(get3d)(hsl, 0, y, x);
      s = THTensor_(get3d)(hsl, 1, y, x);
      l = THTensor_(get3d)(hsl, 2, y, x);

      if(s == 0) {
        // achromatic
        r = l;
        g = l;
        b = l;
      } else {
        real q = (l < 0.5) ? (l * (1 + s)) : (l + s - l * s);
        real p = 2 * l - q;
        real hr = h + 1./3;
        real hg = h;
        real hb = h - 1./3;
        r = image_(hue2rgb)(p, q, hr);
        g = image_(hue2rgb)(p, q, hg);
        b = image_(hue2rgb)(p, q, hb);
      }

      // set rgb
      THTensor_(set3d)(rgb, 0, y, x, r);
      THTensor_(set3d)(rgb, 1, y, x, g);
      THTensor_(set3d)(rgb, 2, y, x, b);
    }
  }
  return 0;
}

/*
 * Converts an RGB color value to HSV. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes r, g, and b are contained in the set [0, 1] and
 * returns h, s, and v in the set [0, 1].
 */
int image_(Main_rgb2hsv)(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *hsv = luaT_checkudata(L, 2, torch_(Tensor_id));

  int y,x;
  real r,g,b,h,s,v;
  for (y=0; y<rgb->size[1]; y++) {
    for (x=0; x<rgb->size[2]; x++) {
      // get Rgb
      r = THTensor_(get3d)(rgb, 0, y, x);
      g = THTensor_(get3d)(rgb, 1, y, x);
      b = THTensor_(get3d)(rgb, 2, y, x);

      real mx = max(max(r, g), b);
      real mn = min(min(r, g), b);
      h = mx;
      v = mx;

      real d = mx - mn;
      s = (mx==0) ? 0 : d/mx;

      if(mx == mn) {
        h = 0; // achromatic
      } else {
        if (mx == r) {
          h = (g - b) / d + (g < b ? 6 : 0);
        } else if (mx == g) {
          h = (b - r) / d + 2; 
        } else {
          h = (r - g) / d + 4;
        }
        h /= 6;
      }

      // set hsv
      THTensor_(set3d)(hsv, 0, y, x, h);
      THTensor_(set3d)(hsv, 1, y, x, s);
      THTensor_(set3d)(hsv, 2, y, x, v);
    }
  }
  return 0;
}

/*
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 1].
 */
int image_(Main_hsv2rgb)(lua_State *L) {
  THTensor *hsv = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *rgb = luaT_checkudata(L, 2, torch_(Tensor_id));

  int y,x;
  real r,g,b,h,s,v;
  for (y=0; y<hsv->size[1]; y++) {
    for (x=0; x<hsv->size[2]; x++) {
      // get hsv
      h = THTensor_(get3d)(hsv, 0, y, x);
      s = THTensor_(get3d)(hsv, 1, y, x);
      v = THTensor_(get3d)(hsv, 2, y, x);

      int i = floor(h*6.);
      real f = h*6-i;
      real p = v*(1-s);
      real q = v*(1-f*s);
      real t = v*(1-(1-f)*s);

      switch (i % 6) {
      case 0: r = v, g = t, b = p; break;
      case 1: r = q, g = v, b = p; break;
      case 2: r = p, g = v, b = t; break;
      case 3: r = p, g = q, b = v; break;
      case 4: r = t, g = p, b = v; break;
      case 5: r = v, g = p, b = q; break;
      default: r=0; g = 0, b = 0; break;
      }

      // set rgb
      THTensor_(set3d)(rgb, 0, y, x, r);
      THTensor_(set3d)(rgb, 1, y, x, g);
      THTensor_(set3d)(rgb, 2, y, x, b);
    }
  }
  return 0;
}

static const struct luaL_Reg image_(Main__) [] = {
  {"scaleSimple", image_(Main_scaleSimple)},
  {"scaleBilinear", image_(Main_scaleBilinear)},
  {"rotate", image_(Main_rotate)},
  {"translate", image_(Main_translate)},
  {"cropNoScale", image_(Main_cropNoScale)},
  {"saturate", image_(Main_saturate)},
  {"rgb2hsv", image_(Main_rgb2hsv)},
  {"rgb2hsl", image_(Main_rgb2hsl)},
  {"hsv2rgb", image_(Main_hsv2rgb)},
  {"hsl2rgb", image_(Main_hsl2rgb)},
  {NULL, NULL}
};

void image_(Main_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, image_(Main__), "image");
}

#endif
