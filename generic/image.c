#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/image.c"
#else

#undef MAX
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b) )

#undef MIN
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

#undef TAPI
#define TAPI __declspec(dllimport)

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

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
    float si_f;
    long si_i;
    float scale = (float)(src_len - 1) / (dst_len - 1);

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
    long si0_i = 0; float si0_f = 0;
    long si1_i; float si1_f;
    long si;
    float scale = (float)src_len / dst_len;
    float acc, n;

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

  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
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
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  float scx, scy;

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
  scx=((float)src_width)/((float)dst_width);
  scy=((float)src_height)/((float)dst_height);

#pragma omp parallel for private(j, i, k)
  for(j = 0; j < dst_height; j++) {
    for(i = 0; i < dst_width; i++) {
      float val = 0.0;
      long ii=(long) (((float)i)*scx);
      long jj=(long) (((float)j)*scy);
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
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
  float theta = luaL_checknumber(L, 3);
  float cos_theta, sin_theta;
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  float xc, yc;
  float id,jd;
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

  sin_theta = sinf(theta);
  cos_theta = cosf(theta);

  for(j = 0; j < dst_height; j++) {
    jd=j;
    for(i = 0; i < dst_width; i++) {
      float val = -1;
      id= i;

      ii=(long)( cos_theta*(id-xc)-sin_theta*(jd-yc) );
      jj=(long)( cos_theta*(jd-yc)+sin_theta*(id-xc) );
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
static int image_(Main_rotateBilinear)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
  float theta = luaL_checknumber(L, 3);
  real *src, *dst;
  long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
  long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
  long i, j, k;
  float xc, yc;
  float id,jd;
  long ii_0, ii_1, jj_0, jj_1;

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
      float val = -1;
      real ri, rj, wi, wj;
      id= i;
      ri = cos(theta)*(id-xc)-sin(theta)*(jd-yc);
      rj = cos(theta)*(jd-yc)+sin(theta)*(id-xc);

      ii_0=(long)floor(ri);
      ii_1=ii_0 + 1;
      jj_0=(long)floor(rj);
      jj_1=jj_0 + 1;
      wi = ri - ii_0;
      wj = rj - jj_0;
      ii_0+=(long) xc; ii_1+=(long) xc; jj_0+=(long) yc;jj_1+=(long) yc;

      /* rotated corners are blank */
      if(ii_1>src_width-1) val=0;
      if(jj_1>src_height-1) val=0;
      if(ii_0<0) val=0;
      if(jj_0<0) val=0;

      if(Tsrc->nDimension==2) {
        if(val==-1)
          val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1]
            + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1]
            + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1]
            + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1];
        dst[i*dst_stride2+j*dst_stride1] = val;
      } else {
        int do_copy=0; if(val==-1) do_copy=1;
        for(k=0;k<src_depth;k++) {
          if(do_copy) {
            val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1+k*src_stride0]
              + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1+k*src_stride0]
              + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1+k*src_stride0]
              + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1+k*src_stride0];
          }
          dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
        }
      }
    }
  }
  return 0;
}

static int image_(Main_polar)(lua_State *L)
{
    THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
    THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
    float doFull = luaL_checknumber(L, 3);
    real *src, *dst;
    long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
    long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
    long i, j, k;
    float id, jd, a, r, m, midY, midX;
    long ii,jj;
    
    luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "polar: src not 2 or 3 dimensional");
    luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "polar: dst not 2 or 3 dimensional");
    
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
    
    if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
        
    if( (Tsrc->nDimension!=Tdst->nDimension) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
            
    // compute maximum distance
    midY = (float) src_height / 2.0;
    midX = (float) src_width  / 2.0;
    if(doFull == 1) {
      m = sqrt((float) src_width * (float) src_width + (float) src_height * (float) src_height) / 2.0;
    }
    else {
      m = (src_width < src_height) ? midX : midY;
    }
    
    // loop to fill polar image
    for(j = 0; j < dst_height; j++) {               // orientation loop
        jd = (float) j;
        a = (2 * M_PI * jd) / (float) dst_height;   // current angle
        for(i = 0; i < dst_width; i++) {            // radius loop
            float val = -1;
            id = (float) i;
            r = (m * id) / (float) dst_width;       // current distance
            
            jj = (long) floor( r * cos(a) + midY);  // y-location in source image
            ii = (long) floor(-r * sin(a) + midX);  // x-location in source image
            
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
static int image_(Main_polarBilinear)(lua_State *L)
{
    THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
    THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
    float doFull = luaL_checknumber(L, 3);
    real *src, *dst;
    long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
    long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
    long i, j, k;
    float id, jd, a, r, m, midY, midX;
    long ii_0, ii_1, jj_0, jj_1;
    
    luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "polar: src not 2 or 3 dimensional");
    luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "polar: dst not 2 or 3 dimensional");
    
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
    
    if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    if( (Tsrc->nDimension!=Tdst->nDimension) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    // compute maximum distance
    midY = (float) src_height / 2.0;
    midX = (float) src_width  / 2.0;
    if(doFull == 1) {
      m = sqrt((float) src_width * (float) src_width + (float) src_height * (float) src_height) / 2.0;
    }
    else {
      m = (src_width < src_height) ? midX : midY;
    }
    
    // loop to fill polar image
    for(j = 0; j < dst_height; j++) {                 // orientation loop
        jd = (float) j;
        a = (2 * M_PI * jd) / (float) dst_height;     // current angle
        for(i = 0; i < dst_width; i++) {              // radius loop
            float val = -1;
            real ri, rj, wi, wj;
            id = (float) i;
            r = (m * id) / (float) dst_width;         // current distance
            
            rj =  r * cos(a) + midY;                  // y-location in source image
            ri = -r * sin(a) + midX;                  // x-location in source image
            
            ii_0=(long)floor(ri);
            ii_1=ii_0 + 1;
            jj_0=(long)floor(rj);
            jj_1=jj_0 + 1;
            wi = ri - ii_0;
            wj = rj - jj_0;
            
            // switch to nearest interpolation when bilinear is impossible
            if(ii_1>src_width-1 || jj_1>src_height-1 || ii_0<0 || jj_0<0) {
                if(ii_0>src_width-1) val=0;
                if(jj_0>src_height-1) val=0;
                if(ii_0<0) val=0;
                if(jj_0<0) val=0;
                
                if(Tsrc->nDimension==2)
                {
                    if(val==-1)
                        val=src[ii_0*src_stride2+jj_0*src_stride1];
                    dst[i*dst_stride2+j*dst_stride1] = val;
                }
                else
                {
                    int do_copy=0; if(val==-1) do_copy=1;
                    for(k=0;k<src_depth;k++)
                    {
                        if(do_copy)
                            val=src[ii_0*src_stride2+jj_0*src_stride1+k*src_stride0];
                        dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
                    }
                }
            }
            
            // bilinear interpolation
            else {
                if(Tsrc->nDimension==2) {
                    if(val==-1)
                        val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1]
                        + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1]
                        + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1]
                        + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1];
                    dst[i*dst_stride2+j*dst_stride1] = val;
                } else {
                    int do_copy=0; if(val==-1) do_copy=1;
                    for(k=0;k<src_depth;k++) {
                        if(do_copy) {
                            val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1+k*src_stride0]
                            + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1+k*src_stride0]
                            + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1+k*src_stride0]
                            + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1+k*src_stride0];
                        }
                        dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
                    }
                }
            }
        }
    }
    return 0;
}

static int image_(Main_logPolar)(lua_State *L)
{
    THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
    THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
    float doFull = luaL_checknumber(L, 3);
    real *src, *dst;
    long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
    long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
    long i, j, k;
    float id, jd, a, r, m, midY, midX, fw;
    long ii,jj;
    
    luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "polar: src not 2 or 3 dimensional");
    luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "polar: dst not 2 or 3 dimensional");
    
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
    
    if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    if( (Tsrc->nDimension!=Tdst->nDimension) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    // compute maximum distance
    midY = (float) src_height / 2.0;
    midX = (float) src_width  / 2.0;
    if(doFull == 1) {
        m = sqrt((float) src_width * (float) src_width + (float) src_height * (float) src_height) / 2.0;
    }
    else {
        m = (src_width < src_height) ? midX : midY;
    }
    
    // loop to fill polar image
    fw = log(m) / (float) dst_width;
    for(j = 0; j < dst_height; j++) {               // orientation loop
        jd = (float) j;
        a = (2 * M_PI * jd) / (float) dst_height;   // current angle
        for(i = 0; i < dst_width; i++) {            // radius loop
            float val = -1;
            id = (float) i;
            
            r = exp(id * fw);
            
            jj = (long) floor( r * cos(a) + midY);  // y-location in source image
            ii = (long) floor(-r * sin(a) + midX);  // x-location in source image
            
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
static int image_(Main_logPolarBilinear)(lua_State *L)
{
    THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
    THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
    float doFull = luaL_checknumber(L, 3);
    real *src, *dst;
    long dst_stride0, dst_stride1, dst_stride2, dst_width, dst_height, dst_depth;
    long src_stride0, src_stride1, src_stride2, src_width, src_height, src_depth;
    long i, j, k;
    float id, jd, a, r, m, midY, midX, fw;
    long ii_0, ii_1, jj_0, jj_1;
    
    luaL_argcheck(L, Tsrc->nDimension==2 || Tsrc->nDimension==3, 1, "polar: src not 2 or 3 dimensional");
    luaL_argcheck(L, Tdst->nDimension==2 || Tdst->nDimension==3, 2, "polar: dst not 2 or 3 dimensional");
    
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
    
    if( Tsrc->nDimension==3 && Tdst->nDimension==3 && ( src_depth!=dst_depth) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    if( (Tsrc->nDimension!=Tdst->nDimension) ) {
        luaL_error(L, "image.polar: src and dst depths do not match"); }
    
    // compute maximum distance
    midY = (float) src_height / 2.0;
    midX = (float) src_width  / 2.0;
    if(doFull == 1) {
        m = sqrt((float) src_width * (float) src_width + (float) src_height * (float) src_height) / 2.0;
    }
    else {
        m = (src_width < src_height) ? midX : midY;
    }
    
    // loop to fill polar image
    fw = log(m) / (float) dst_width;
    for(j = 0; j < dst_height; j++) {                 // orientation loop
        jd = (float) j;
        a = (2 * M_PI * jd) / (float) dst_height;     // current angle
        for(i = 0; i < dst_width; i++) {              // radius loop
            float val = -1;
            real ri, rj, wi, wj;
            id = (float) i;
            
            r = exp(id * fw);
            
            rj =  r * cos(a) + midY;                  // y-location in source image
            ri = -r * sin(a) + midX;                  // x-location in source image
            
            ii_0=(long)floor(ri);
            ii_1=ii_0 + 1;
            jj_0=(long)floor(rj);
            jj_1=jj_0 + 1;
            wi = ri - ii_0;
            wj = rj - jj_0;
            
            // switch to nearest interpolation when bilinear is impossible
            if(ii_1>src_width-1 || jj_1>src_height-1 || ii_0<0 || jj_0<0) {
                if(ii_0>src_width-1) val=0;
                if(jj_0>src_height-1) val=0;
                if(ii_0<0) val=0;
                if(jj_0<0) val=0;
                
                if(Tsrc->nDimension==2)
                {
                    if(val==-1)
                        val=src[ii_0*src_stride2+jj_0*src_stride1];
                    dst[i*dst_stride2+j*dst_stride1] = val;
                }
                else
                {
                    int do_copy=0; if(val==-1) do_copy=1;
                    for(k=0;k<src_depth;k++)
                    {
                        if(do_copy)
                            val=src[ii_0*src_stride2+jj_0*src_stride1+k*src_stride0];
                        dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
                    }
                }
            }
            
            // bilinear interpolation
            else {
                if(Tsrc->nDimension==2) {
                    if(val==-1)
                        val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1]
                        + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1]
                        + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1]
                        + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1];
                    dst[i*dst_stride2+j*dst_stride1] = val;
                } else {
                    int do_copy=0; if(val==-1) do_copy=1;
                    for(k=0;k<src_depth;k++) {
                        if(do_copy) {
                            val = (1.0 - wi) * (1.0 - wj) * src[ii_0*src_stride2+jj_0*src_stride1+k*src_stride0]
                            + wi * (1.0 - wj) * src[ii_1*src_stride2+jj_0*src_stride1+k*src_stride0]
                            + (1.0 - wi) * wj * src[ii_0*src_stride2+jj_1*src_stride1+k*src_stride0]
                            + wi * wj * src[ii_1*src_stride2+jj_1*src_stride1+k*src_stride0];
                        }
                        dst[i*dst_stride2+j*dst_stride1+k*dst_stride0] = val;
                    }
                }
            }
        }
    }
    return 0;
}


static int image_(Main_cropNoScale)(lua_State *L)
{
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
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
      float val = 0.0;

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
  THTensor *Tsrc = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *Tdst = luaT_checkudata(L, 2, torch_Tensor);
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

  dst_stride0 = 1;
  dst_stride1 = Tdst->stride[Tdst->nDimension-2];
  dst_stride2 = Tdst->stride[Tdst->nDimension-1];
  dst_depth =  1;
  dst_height = Tdst->size[Tdst->nDimension-2];
  dst_width = Tdst->size[Tdst->nDimension-1];
  if(Tdst->nDimension == 3) {
    dst_stride0 = Tdst->stride[0];
    dst_depth = Tdst->size[0];
  }

  src_stride0 = 1;
  src_stride1 = Tsrc->stride[Tsrc->nDimension-2];
  src_stride2 = Tsrc->stride[Tsrc->nDimension-1];
  src_depth =  1;
  src_height = Tsrc->size[Tsrc->nDimension-2];
  src_width = Tsrc->size[Tsrc->nDimension-1];
  if(Tsrc->nDimension == 3) {
    src_stride0 = Tsrc->stride[0];
    src_depth = Tsrc->size[0];
  }

  if( Tdst->nDimension==3 && ( src_depth!=dst_depth) )
    luaL_error(L, "image.translate: src and dst depths do not match");

  for(j = 0; j < src_height; j++) {
    for(i = 0; i < src_width; i++) {
      long ii=i+shiftx;
      long jj=j+shifty;

      // Check it's within destination bounds, else crop
      if(ii<dst_width && jj<dst_height && ii>=0 && jj>=0) {
        for(k=0;k<src_depth;k++) {
          dst[ii*dst_stride2+jj*dst_stride1+k*dst_stride0] = src[i*src_stride2+j*src_stride1+k*src_stride0];
        }
      }
    }
  }
  return 0;
}

static int image_(Main_saturate)(lua_State *L) {
  THTensor *input = luaT_checkudata(L, 1, torch_Tensor);
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
  THTensor *rgb = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *hsl = luaT_checkudata(L, 2, torch_Tensor);

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
  THTensor *hsl = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *rgb = luaT_checkudata(L, 2, torch_Tensor);

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
  THTensor *rgb = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *hsv = luaT_checkudata(L, 2, torch_Tensor);

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
  THTensor *hsv = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *rgb = luaT_checkudata(L, 2, torch_Tensor);

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

/*
 * Converts an sRGB color value to LAB.
 * Based on http://www.brucelindbloom.com/index.html?Equations.html.
 * Assumes r, g, and b are contained in the set [0, 1].
 * LAB output is NOT restricted to [0, 1]!
 */
int image_(Main_rgb2lab)(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *lab = luaT_checkudata(L, 2, torch_Tensor);

  // CIE Standard
  double epsilon = 216.0/24389.0;
  double k = 24389.0/27.0;
  // D65 white point
  double xn = 0.950456;
  double zn = 1.088754;

  int y,x;
  real r,g,b,l,a,_b;
  for (y=0; y<rgb->size[1]; y++) {
    for (x=0; x<rgb->size[2]; x++) {
      // get RGB
      r = gamma_expand_sRGB(THTensor_(get3d)(rgb, 0, y, x));
      g = gamma_expand_sRGB(THTensor_(get3d)(rgb, 1, y, x));
      b = gamma_expand_sRGB(THTensor_(get3d)(rgb, 2, y, x));

      // sRGB to XYZ
      double X = 0.412453 * r + 0.357580 * g + 0.180423 * b;
      double Y = 0.212671 * r + 0.715160 * g + 0.072169 * b;
      double Z = 0.019334 * r + 0.119193 * g + 0.950227 * b;

      // normalize for D65 white point
      X /= xn;
      Z /= zn;

      // XYZ normalized to CIE Lab
      double fx = X > epsilon ? pow(X, 1/3.0) : (k * X + 16)/116;
      double fy = Y > epsilon ? pow(Y, 1/3.0) : (k * Y + 16)/116;
      double fz = Z > epsilon ? pow(Z, 1/3.0) : (k * Z + 16)/116;
      l = 116 * fy - 16;
      a = 500 * (fx - fy);
      _b = 200 * (fy - fz);

      // set lab
      THTensor_(set3d)(lab, 0, y, x, l);
      THTensor_(set3d)(lab, 1, y, x, a);
      THTensor_(set3d)(lab, 2, y, x, _b);
    }
  }
  return 0;
}

/*
 * Converts an LAB color value to sRGB.
 * Based on http://www.brucelindbloom.com/index.html?Equations.html.
 * returns r, g, and b in the set [0, 1].
 */
int image_(Main_lab2rgb)(lua_State *L) {
  THTensor *lab = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *rgb = luaT_checkudata(L, 2, torch_Tensor);

  int y,x;
  real r,g,b,l,a,_b;

  // CIE Standard
  double epsilon = 216.0/24389.0;
  double k = 24389.0/27.0;
  // D65 white point
  double xn = 0.950456;
  double zn = 1.088754;

  for (y=0; y<lab->size[1]; y++) {
    for (x=0; x<lab->size[2]; x++) {
      // get lab
      l = THTensor_(get3d)(lab, 0, y, x);
      a = THTensor_(get3d)(lab, 1, y, x);
      _b = THTensor_(get3d)(lab, 2, y, x);

      // LAB to XYZ
      double fy = (l + 16) / 116;
      double fz = fy - _b / 200;
      double fx = (a / 500) + fy;
      double X = pow(fx, 3);
      if (X <= epsilon)
        X = (116 * fx - 16) / k;
      double Y = l > (k * epsilon) ? pow((l + 16) / 116, 3) : l/k;
      double Z = pow(fz, 3);
      if (Z <= epsilon)
        Z = (116 * fz - 16) / k;

      X *= xn;
      Z *= zn;

      // XYZ to sRGB
      r =  3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z;
      g = -0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z;
      b =  0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z;

      // set rgb
      THTensor_(set3d)(rgb, 0, y, x, gamma_compress_sRGB(r));
      THTensor_(set3d)(rgb, 1, y, x, gamma_compress_sRGB(g));
      THTensor_(set3d)(rgb, 2, y, x, gamma_compress_sRGB(b));
    }
  }
  return 0;
}

/* Vertically flip an image */
int image_(Main_vflip)(lua_State *L) {
  THTensor *dst = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *src = luaT_checkudata(L, 2, torch_Tensor);

  int width = dst->size[2];
  int height = dst->size[1];
  int src_width = src->size[2];
  int src_height = src->size[1];
  int channels = dst->size[0];
  long *is = src->stride;
  long *os = dst->stride;

  // get raw pointers
  real *dst_data = THTensor_(data)(dst);
  real *src_data = THTensor_(data)(src);

  long k, x, y;
  if (dst_data != src_data) {
      /* not in-place.
       * this branch could be removed by first duplicating the src into dst then doing inplace */
      for(k=0; k<channels; k++) {
          for (y=0; y<height; y++) {
            for (x=0; x<width; x++) {
                dst_data[ k*os[0] + (height-1-y)*os[1] + x*os[2] ] = src_data[ k*is[0] + y*is[1] + x*is[2] ];
            }
          }
      }
  } else {
      /* in-place  */
      real swap, * src_px,  * dst_px;
      long half_height = height >> 1;
      for(k=0; k<channels; k++) {
          for (y=0; y < half_height; y++) {
            for (x=0; x<width; x++) {
                src_px = src_data + k*is[0] + y*is[1] + x*is[2];
                dst_px =  dst_data + k*is[0] + (height-1-y)*is[1] + x*is[2];
                swap = *dst_px;
                *dst_px = *src_px;
                *src_px = swap;
            }
          }
      }
  }

  return 0;
}


/* Horizontally flip an image */
int image_(Main_hflip)(lua_State *L) {
  THTensor *dst = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *src = luaT_checkudata(L, 2, torch_Tensor);

  int width = dst->size[2];
  int height = dst->size[1];
  int src_width = src->size[2];
  int src_height = src->size[1];
  int channels = dst->size[0];
  long *is = src->stride;
  long *os = dst->stride;

  // get raw pointers
  real *dst_data = THTensor_(data)(dst);
  real *src_data = THTensor_(data)(src);

  long k, x, y;
  if (dst_data != src_data) {
      /* not in-place.
       * this branch could be removed by first duplicating the src into dst then doing inplace */
      for(k=0; k<channels; k++) {
          for (y=0; y<height; y++) {
              for (x=0; x<width; x++) {
                  dst_data[ k*os[0] + y*os[1] + (width-x-1)*os[2] ] = src_data[ k*is[0] + y*is[1] + x*is[2] ];
              }
          }
      }
  } else {
      /* in-place  */
      real swap, * src_px,  * dst_px;
      long half_width = width >> 1;
      for(k=0; k<channels; k++) {
          for (y=0; y < height; y++) {
            for (x=0; x<half_width; x++) {
                src_px = src_data + k*is[0] + y*is[1] + x*is[2];
                dst_px =  dst_data + k*is[0] + y*is[1] + (width-x-1)*is[2];
                swap = *dst_px;
                *dst_px = *src_px;
                *src_px = swap;
            }
          }
      }
  }

  return 0;
}

/* flip an image along a specified dimension */
int image_(Main_flip)(lua_State *L) {
  THTensor *dst = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *src = luaT_checkudata(L, 2, torch_Tensor);
  long flip_dim = luaL_checklong(L, 3);
  
  if (dst->nDimension != src->nDimension) {
    luaL_error(L, "image.flip: src and dst nDimension does not match");
  }
  
  if (flip_dim < 1 || flip_dim > dst->nDimension) {
    luaL_error(L, "image.flip: flip_dim out of bounds");
  }
  flip_dim--;  //  Make it zero indexed

  // get raw pointers
  real *dst_data = THTensor_(data)(dst);
  real *src_data = THTensor_(data)(src);
  if (dst_data == src_data) {  
    luaL_error(L, "image.flip: in-place flip not supported");
  }
  
  long size0 = dst->size[0];
  long size1 = dst->size[1];
  long size2 = dst->size[2];
  long size3 = dst->size[3];
  long size4 = dst->size[4];
  long size_flip = dst->size[flip_dim];
  
  if (src->size[0] != size0 || src->size[1] != size1 || 
      src->size[2] != size2 || src->size[3] != size3 || 
      src->size[4] != size4) {
    luaL_error(L, "image.flip: src and dst are not the same size");
  }
  
  long *is = src->stride;
  long *os = dst->stride;

  long x, y, z, d, t, isrc, idst;
  for (t = 0; t < size0; t++) {
    for (d = 0; d < size1; d++) {
      for (z = 0; z < size2; z++) {
        for (y = 0; y < size3; y++) {
          for (x = 0; x < size4; x++) {
            isrc = t*is[0] + d*is[1] + z*is[2] + y*is[3] + x*is[4];
            // The big switch statement here looks ugly, however on my machine
            // gcc compiles it to a skip list, so it should be fast.
            switch (flip_dim) {
              case 0:
                idst = (size0 - t - 1)*os[0] + d*os[1] + z*os[2] + y*os[3] + x*os[4];
                break;
              case 1:
                idst = t*os[0] + (size1 - d - 1)*os[1] + z*os[2] + y*os[3] + x*os[4];
                break;
              case 2:
                idst = t*os[0] + d*os[1] + (size2 - z - 1)*os[2] + y*os[3] + x*os[4];
                break;
              case 3:
                idst = t*os[0] + d*os[1] + z*os[2] + (size3 - y - 1)*os[3] + x*os[4];
                break;
              case 4:
                idst = t*os[0] + d*os[1] + z*os[2] + y*os[3] + (size4 - x - 1)*os[4];
                break;
            }              
            dst_data[ idst ] = src_data[  isrc ];
          }
        }
      }
    }
  }

  return 0;
}

/*
 * Warps an image, according to an (x,y) flow field. The flow
 * field is in the space of the destination image, each vector
 * ponts to a source pixel in the original image.
 */
int image_(Main_warp)(lua_State *L) {
  THTensor *dst = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *src = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *flowfield = luaT_checkudata(L, 3, torch_Tensor);
  int mode = lua_tointeger(L, 4);
  int offset_mode = lua_toboolean(L, 5);
  int clamp_mode = lua_tointeger(L, 6);

  // dims
  int width = dst->size[2];
  int height = dst->size[1];
  int src_width = src->size[2];
  int src_height = src->size[1];
  int channels = dst->size[0];
  long *is = src->stride;
  long *os = dst->stride;
  long *fs = flowfield->stride;

  // get raw pointers
  real *dst_data = THTensor_(data)(dst);
  real *src_data = THTensor_(data)(src);
  real *flow_data = THTensor_(data)(flowfield);

  // resample
  long k,x,y,jj,v,u,i,j;
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      // subpixel position:
      real flow_y = flow_data[ 0*fs[0] + y*fs[1] + x*fs[2] ];
      real flow_x = flow_data[ 1*fs[0] + y*fs[1] + x*fs[2] ];
      float iy = offset_mode*y + flow_y;
      float ix = offset_mode*x + flow_x;

      // borders
      int off_image = 0;
      if (iy < 0 || iy > src_height - 1 ||
          ix < 0 || ix > src_width - 1) {
        off_image = 1;
      }

      if (off_image == 1 && clamp_mode == 1) {
        // We're off the image and we're clamping the input image to 0
        for (k=0; k<channels; k++) {
          dst_data[ k*os[0] + y*os[1] + x*os[2] ] = 0;
        }
      } else {
        ix = MAX(ix,0); ix = MIN(ix,src_width-1);
        iy = MAX(iy,0); iy = MIN(iy,src_height-1);

        // bilinear?
        switch (mode) {
        case 1:  // Bilinear interpolation
          {
            // 4 nearest neighbors:
            long ix_nw = floor(ix);
            long iy_nw = floor(iy);
            long ix_ne = ix_nw + 1;
            long iy_ne = iy_nw;
            long ix_sw = ix_nw;
            long iy_sw = iy_nw + 1;
            long ix_se = ix_nw + 1;
            long iy_se = iy_nw + 1;

            // get surfaces to each neighbor:
            real nw = ((real)(ix_se-ix))*(iy_se-iy);
            real ne = ((real)(ix-ix_sw))*(iy_sw-iy);
            real sw = ((real)(ix_ne-ix))*(iy-iy_ne);
            real se = ((real)(ix-ix_nw))*(iy-iy_nw);

            // weighted sum of neighbors:
            for (k=0; k<channels; k++) {
              dst_data[ k*os[0] + y*os[1] + x*os[2] ] =
                  src_data[ k*is[0] +               iy_nw*is[1] +              ix_nw*is[2] ] * nw
                + src_data[ k*is[0] +               iy_ne*is[1] + MIN(ix_ne,src_width-1)*is[2] ] * ne
                + src_data[ k*is[0] + MIN(iy_sw,src_height-1)*is[1] +              ix_sw*is[2] ] * sw
                + src_data[ k*is[0] + MIN(iy_se,src_height-1)*is[1] + MIN(ix_se,src_width-1)*is[2] ] * se;
            }
          }
	      break;
        case 0:  // Simple (i.e., nearest neighbor)
          {
            // 1 nearest neighbor:
            long ix_n = floor(ix+0.5);
            long iy_n = floor(iy+0.5);

            // weighted sum of neighbors:
            for (k=0; k<channels; k++) {
              dst_data[ k*os[0] + y*os[1] + x*os[2] ] = src_data[ k*is[0] + iy_n*is[1] + ix_n*is[2] ];
            }
          }
          break;
        case 2:  // Bicubic
          {
	        // Calculate fractional and integer components
            long x_pix = floor(ix);
            long y_pix = floor(iy);
            real dx = ix - (real)x_pix;
            real dy = iy - (real)y_pix;

            real C[4];
            for (k=0; k<channels; k++) {
              // Sweep by rows through the samples (to calculate final cubic coefs)
              for (jj = 0; jj <= 3; jj++) {
                v = y_pix - 1 + jj;
                // We need to clamp all uv values to image border: hopefully
                // branch prediction and compiler reordering takes care of all
                // the conditionals (since the branch probabilities are heavily
                // skewed).  Alternatively an inline "getPixelSafe" function would
                // would be clearer here, but cannot be done with lua?
                v = MAX(MIN((long)(src_height-1), v), 0);
                long ofst = k * is[0] + v * is[1];
                u = x_pix;
                u = MAX(MIN((long)(src_width-1), u), 0);
                real a0 = src_data[ofst + u * is[2]];
                u = x_pix - 1;
                u = MAX(MIN((long)(src_width-1), u), 0);
                real d0 = src_data[ofst + u * is[2]] - a0;
                u = x_pix + 1;
                u = MAX(MIN((long)(src_width-1), u), 0);
                real d2 = src_data[ofst + u * is[2]] - a0;
                u = x_pix + 2;
                u = MAX(MIN((long)(src_width-1), u), 0);
                real d3 = src_data[ofst + u * is[2]] - a0;

                // Note: there are mostly static casts, optimizer will take care of
                // of it for us (prevents compiler warnings in new gcc)
                real a1 =  -(real)1/(real)3*d0 + d2 -(real)1/(real)6*d3;
                real a2 = (real)1/(real)2*d0 + (real)1/(real)2*d2;
                real a3 = -(real)1/(real)6*d0 - (real)1/(real)2*d2 +
                  (real)1/(real)6*d3;
                C[jj] = a0 + dx * (a1 + dx * (a2 + a3 * dx));
              }

              real d0 = C[0]-C[1];
              real d2 = C[2]-C[1];
              real d3 = C[3]-C[1];
              real a0 = C[1];
              real a1 = -(real)1/(real)3*d0 + d2 - (real)1/(real)6*d3;
              real a2 = (real)1/(real)2*d0 + (real)1/(real)2*d2;
              real a3 = -(real)1/(real)6*d0 - (real)1/(real)2*d2 +
                (real)1/(real)6*d3;
              real Cc = a0 + dy * (a1 + dy * (a2 + a3 * dy));

              // I assume that since the image is stored as reals we don't have
              // to worry about clamping to min and max int (to prevent over or
              // underflow)
              dst_data[ k*os[0] + y*os[1] + x*os[2] ] = Cc;
	        }
          }
          break;
        case 3:  // Lanczos
	      {
            // Note: Lanczos can be made fast if the resampling period is
            // constant... and therefore the Lu, Lv can be cached and reused.
            // However, unfortunately warp makes no assumptions about resampling
            // and so we need to perform the O(k^2) convolution on each pixel AND
            // we have to re-calculate the kernel for every pixel.
            // See wikipedia for more info.
            // It is however an extremely good approximation to to full sinc
            // interpolation (IIR) filter.
            // Another note is that the version here has been optimized using
            // pretty aggressive code flow and explicit inlining.  It might not
            // be very readable (contact me, Jonathan Tompson, if it is not)

            // Calculate fractional and integer components
            long x_pix = floor(ix);
            long y_pix = floor(iy);

            // Precalculate the L(x) function evaluations in the u and v direction
            const long rad = 3;  // This is a tunable parameter: 2 to 3 is OK
            float Lu[2 * rad];  // L(x) for u direction
            float Lv[2 * rad];  // L(x) for v direction
            for (u=x_pix-rad+1, i=0; u<=x_pix+rad; u++, i++) {
              float du = ix - (float)u;  // Lanczos kernel x value
              du = du < 0 ? -du : du;  // prefer not to used std absf
              if (du < 0.000001f) {  // TODO: Is there a real eps standard?
                Lu[i] = 1;
              } else if (du > (float)rad) {
                Lu[i] = 0;
              } else {
                Lu[i] = ((float)rad * sin((float)M_PI * du) *
                  sin((float)M_PI * du / (float)rad)) /
                  ((float)(M_PI * M_PI) * du * du);
              }
            }
            for (v=y_pix-rad+1, i=0; v<=y_pix+rad; v++, i++) {
              float dv = iy - (float)v;  // Lanczos kernel x value
              dv = dv < 0 ? -dv : dv;  // prefer not to used std absf
              if (dv < 0.000001f) {  // TODO: Is there a real eps standard?
                Lv[i] = 1;
              } else if (dv > (float)rad) {
                Lv[i] = 0;
              } else {
                Lv[i] = ((float)rad * sin((float)M_PI * dv) *
                  sin((float)M_PI * dv / (float)rad)) /
                  ((float)(M_PI * M_PI) * dv * dv);
              }
            }
            float sum_weights = 0;
            for (u=0; u<2*rad; u++) {
              for (v=0; v<2*rad; v++) {
                sum_weights += (Lu[u] * Lv[v]);
              }
            }

            for (k=0; k<channels; k++) {
              real result = 0;
              for (u=x_pix-rad+1, i=0; u<=x_pix+rad; u++, i++) {
                long curu = MAX(MIN((long)(src_width-1), u), 0);
                for (v=y_pix-rad+1, j=0; v<=y_pix+rad; v++, j++) {
                  long curv = MAX(MIN((long)(src_height-1), v), 0);
                  real Suv = src_data[k * is[0] + curv * is[1] + curu * is[2]];

                  real weight = (real)(Lu[i] * Lv[j]);
                  result += (Suv * weight);
                }
              }
              // Normalize by the sum of the weights
              result = result / (float)sum_weights;

              // Again,  I assume that since the image is stored as reals we
              // don't have to worry about clamping to min and max int (to
              // prevent over or underflow)
              dst_data[ k*os[0] + y*os[1] + x*os[2] ] = result;
            }
          }
          break;
        }  // end switch (mode)
      }  // end else
    }
  }

  // done
  return 0;
}

int image_(Main_gaussian)(lua_State *L) {
  THTensor *dst = luaT_checkudata(L, 1, torch_Tensor);
  long width = dst->size[1];
  long height = dst->size[0];
  long *os = dst->stride;

  real *dst_data = THTensor_(data)(dst);

  real amplitude = (real)lua_tonumber(L, 2);
  int normalize = (int)lua_toboolean(L, 3);
  real sigma_u = (real)lua_tonumber(L, 4);
  real sigma_v = (real)lua_tonumber(L, 5);
  real mean_u = (real)lua_tonumber(L, 6) * (real)width + (real)0.5;
  real mean_v = (real)lua_tonumber(L, 7) * (real)height + (real)0.5;

  // Precalculate 1/(sigma*size) for speed (for some stupid reason the pragma
  // omp declaration prevents gcc from optimizing the inside loop on my macine:
  // verified by checking the assembly output)
  real over_sigmau = (real)1.0 / (sigma_u * (real)width);
  real over_sigmav = (real)1.0 / (sigma_v * (real)height);

  long v, u;
  real du, dv;
#pragma omp parallel for private(v, u, du, dv)
  for (v = 0; v < height; v++) {
    for (u = 0; u < width; u++) {
      du = ((real)u + 1 - mean_u) * over_sigmau;
      dv = ((real)v + 1 - mean_v) * over_sigmav;
      dst_data[ v*os[0] + u*os[1] ] = amplitude *
        exp(-((du*du*0.5) + (dv*dv*0.5)));
    }
  }

  if (normalize) {
    real sum = 0;
    // We could parallelize this, but it's more trouble than it's worth
    for(v = 0; v < height; v++) {
      for(u = 0; u < width; u++) {
        sum += dst_data[ v*os[0] + u*os[1] ];
      }
    }
    real one_over_sum = 1.0 / sum;
#pragma omp parallel for private(v, u)
    for(v = 0; v < height; v++) {
      for(u = 0; u < width; u++) {
        dst_data[ v*os[0] + u*os[1] ] *= one_over_sum;
      }
    }
  }
  return 0;
}

/*
 * Borrowed from github.com/clementfarabet/lua---imgraph
 * with Clément's permission for implementing y2jet()
 */
int image_(Main_colorize)(lua_State *L) {
  // get args
  THTensor *output = (THTensor *)luaT_checkudata(L, 1, torch_Tensor);
  THTensor *input = (THTensor *)luaT_checkudata(L, 2, torch_Tensor);
  THTensor *colormap = (THTensor *)luaT_checkudata(L, 3, torch_Tensor);

  // dims
  long height = input->size[0];
  long width = input->size[1];

  // generate color map if not given
  if (THTensor_(nElement)(colormap) == 0) {
    THTensor_(resize2d)(colormap, width*height, 3);
    THTensor_(fill)(colormap, -1);
  }

  // colormap channels
  int channels = colormap->size[1];

  // generate output
  THTensor_(resize3d)(output, channels, height, width);
  int x,y,k;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      int id = THTensor_(get2d)(input, y, x);
      real check = THTensor_(get2d)(colormap, id, 0);
      if (check == -1) {
        for (k = 0; k < channels; k++) {
          THTensor_(set2d)(colormap, id, k, ((float)rand()/(float)RAND_MAX));
        }
      }
      for (k = 0; k < channels; k++) {
        real color = THTensor_(get2d)(colormap, id, k);
        THTensor_(set3d)(output, k, y, x, color);
      }
    }
  }

  // return nothing
  return 0;
}

int image_(Main_rgb2y)(lua_State *L) {
  THTensor *rgb = luaT_checkudata(L, 1, torch_Tensor);
  THTensor *yim = luaT_checkudata(L, 2, torch_Tensor);

  luaL_argcheck(L, rgb->nDimension == 3, 1, "image.rgb2y: src not 3D");
  luaL_argcheck(L, yim->nDimension == 2, 2, "image.rgb2y: dst not 2D");
  luaL_argcheck(L, rgb->size[1] == yim->size[0], 2,
		"image.rgb2y: src and dst not of same height");
  luaL_argcheck(L, rgb->size[2] == yim->size[1], 2,
		"image.rgb2y: src and dst not of same width");

  int y,x;
  real r,g,b,yc;
  const int height = rgb->size[1];
  const int width = rgb->size[2];
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      // get Rgb
      r = THTensor_(get3d)(rgb, 0, y, x);
      g = THTensor_(get3d)(rgb, 1, y, x);
      b = THTensor_(get3d)(rgb, 2, y, x);

      yc = (real) ((0.299 * (float) r)
		   + (0.587 * (float) g)
		   + (0.114 * (float) b));
      THTensor_(set2d)(yim, y, x, yc);
    }
  }
  return 0;
}

static const struct luaL_Reg image_(Main__) [] = {
  {"scaleSimple", image_(Main_scaleSimple)},
  {"scaleBilinear", image_(Main_scaleBilinear)},
  {"rotate", image_(Main_rotate)},
  {"rotateBilinear", image_(Main_rotateBilinear)},
  {"polar", image_(Main_polar)},
  {"polarBilinear", image_(Main_polarBilinear)},
  {"logPolar", image_(Main_logPolar)},
  {"logPolarBilinear", image_(Main_logPolarBilinear)},
  {"translate", image_(Main_translate)},
  {"cropNoScale", image_(Main_cropNoScale)},
  {"warp", image_(Main_warp)},
  {"saturate", image_(Main_saturate)},
  {"rgb2y",   image_(Main_rgb2y)},
  {"rgb2hsv", image_(Main_rgb2hsv)},
  {"rgb2hsl", image_(Main_rgb2hsl)},
  {"hsv2rgb", image_(Main_hsv2rgb)},
  {"hsl2rgb", image_(Main_hsl2rgb)},
  {"rgb2lab", image_(Main_rgb2lab)},
  {"lab2rgb", image_(Main_lab2rgb)},
  {"gaussian", image_(Main_gaussian)},
  {"vflip", image_(Main_vflip)},
  {"hflip", image_(Main_hflip)},
  {"flip", image_(Main_flip)},
  {"colorize", image_(Main_colorize)},
  {NULL, NULL}
};

void image_(Main_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, image_(Main__), "image");
}

#endif
