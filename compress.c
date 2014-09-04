#include <TH.h>
#include <luaT.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

#define byte unsigned char

struct mem_buffer
{
    byte* buffer;
    size_t read_write_index, size;
};

static void libcompress_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct mem_buffer* p = (struct mem_buffer*)png_get_io_ptr(png_ptr);
    if(p->read_write_index + length > p->size)
        THError("libcompress.decompress.png_read_data: Tried to read past the end of a buffer!");
    memcpy(data, p->buffer + p->read_write_index, length);
    p->read_write_index = p->read_write_index + length;
}

static void libcompress_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct mem_buffer* p=(struct mem_buffer*)png_get_io_ptr(png_ptr);
    size_t total_length = p->read_write_index + length;

    /* If the allocator starts being silly, this could be a really
       inefficient way of dynamically sizing the buffer!
       In practice, the png compression dominates memory reallocation. */
    if(total_length > p->size)
    {
        size_t new_size = total_length;
        p->buffer = (unsigned char*)realloc(p->buffer, new_size);
        p->size = new_size;
    }

    if(!p->buffer)
        THError("libcompress.compress.png_write_data: Failed to allocate buffer!");


    /* copy new bytes to end of buffer */
    memcpy(p->buffer + p->read_write_index, data, length);
    p->read_write_index = total_length;
}

/* The libpng specification specifies that if providing your own write_fn,
you need to provide a flush fn too, but this never gets called.*/
static void libcompress_flush(png_structp png_ptr) { }

/* Useful for when we have some kind of error. */
static void tidyup_write(png_structpp structpp, png_infopp infopp, png_bytep* row_pointers, THByteTensor* tensor)
{
    /* Tidy up the objects we used for the libpng session */
    png_destroy_write_struct(structpp, infopp);

    if(row_pointers) free(row_pointers);

    /* Must now free (or reduce the reference count of) our contiguous tensor. */
    if(tensor) THByteTensor_free(tensor);
}

/* Pack the underlying tensor data from a Tensor into a PNG string.
We don't attempt to save any space/work if the Tensor describes an odd view of a storage
We assume that the most 2 contiguous dimensions of the storage describe an image,
i.e. That a k x m x n Tensor describes k images.
We collapse any higher dimensions whilst compressing a tensor with >2 dimensions.
this is equivalent to vertically 'stacking' each image in the Tensor.
This means that e.g. a 3 x m x n colour image as loaded by image.load will be
compressed completely differently by this function compared to it's representation in file. */
static THByteStorage* libcompress_pack_png_string(THByteTensor* image_tensor)
{
    /* libpng needs each row to be contiguous.
    We also assume every thing is contiguous when populating row_pointers below.
    If the tensor is contiguous, this doesn't do any extra work.
    Note: we need to free this later. */
    THByteTensor* tensorc = THByteTensor_newContiguous(image_tensor);
    byte* tensor_data = THByteTensor_data(tensorc);

    /* A 2D tensor is an image, so we can just compress it.
    We collapse any higher dimensional tensor to 2D
    equivalent to stacking each 2D plane to give a very tall image.*/
    size_t width = tensorc->size[tensorc->nDimension-1];
    size_t height = 1;
    size_t i;
    for(i = 0; i < tensorc->nDimension-1; ++i)
        height = height*tensorc->size[i];

    png_bytep* row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
    const size_t row_stride = width;
    for(i = 0; i < height; ++i)
        row_pointers[i] = &tensor_data[tensorc->storageOffset + i*row_stride];

    /* The is object will be a simple container to hold the compressed data written out by libpng
    We will wrap it in a THByteStorage later */
    struct mem_buffer compressed_image;
    compressed_image.buffer = NULL;
    compressed_image.size = 0;
    compressed_image.read_write_index = 0;

    /* Write out the image tensor in to our buffer using libpng. */
    png_structp write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!write_ptr) {
      tidyup_write(&write_ptr, NULL, row_pointers, tensorc);
      THError("libcompress.compress: couldn't create png_write_struct");
    }

    png_infop write_info_ptr = png_create_info_struct(write_ptr);
    if (!write_info_ptr) {
      tidyup_write(&write_ptr, &write_info_ptr, row_pointers, tensorc);
      THError("libcompress.compress: couldn't create png_info_struct");
    }

    png_set_write_fn(write_ptr, &compressed_image, libcompress_write_data, libcompress_flush);

    /* TODO: Experiment with non-default libpng options */
    png_set_IHDR(write_ptr, write_info_ptr, width, height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(write_ptr, write_info_ptr);
    png_write_image(write_ptr, row_pointers);

    tidyup_write(&write_ptr, &write_info_ptr, row_pointers, tensorc);

    /* The byte storage now assumes control of the memory buffer. */
    THByteStorage* png_string = THByteStorage_newWithData(compressed_image.buffer, compressed_image.size);
    return png_string;
}

static THByteTensor* libcompress_unpack_png_string(THByteStorage* packed_data, THByteTensor* image_tensor)
{
    /* Set up struct to allow libpng to read from the THByteStorage of compressed data. */
    struct mem_buffer compressed_image;
    compressed_image.buffer = packed_data->data;
    compressed_image.size = packed_data->size;
    compressed_image.read_write_index = 0;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) THError("libcompress.decompress: couldn't create png_read_struct");

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        THError("libcompress.decompress: couldn't create png_info_struct");
    }

    /* the code in this if statement gets called if libpng encounters an error. */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        THError("libcompress.decompress: libpng error.");
    }

    png_set_read_fn(png_ptr, &compressed_image, libcompress_read_data);

    /* Read all the png header info up to the image data. */
    png_read_info(png_ptr, info_ptr);
    int width, height, bit_depth, colour_type;
    png_uint_32 png_width, png_height;
    png_get_IHDR(png_ptr, info_ptr, &png_width, &png_height, &bit_depth, &colour_type, 0, 0, 0);
    width = png_width;
    height = png_height;

    const int expected_size = THByteTensor_nElement(image_tensor);
    if(width * height != expected_size) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        THError("libcompress.decompress: Packed tensor size does not match expected size.");
    }

    if(!THByteTensor_isContiguous(image_tensor)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        THError("libcompress.decompress: Cannot decompress into non-contiguous Tensor");
    }

    /* Make libpng decompress directly into the tensor storage. */
    byte* tensor_data = THByteTensor_data(image_tensor);
    png_bytep* row_pointers = (png_bytep*)malloc(height * sizeof(png_bytep));
    const size_t row_stride = width;
    size_t i;
    for(i = 0; i < height; ++i)
        row_pointers[i] = &tensor_data[image_tensor->storageOffset + i*row_stride];

    /* the code in this if statement gets called if libpng encounters an error. */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(row_pointers);
        THError("libcompress.decompress: libpng error.");
    }

    png_read_image(png_ptr, row_pointers);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(row_pointers);

    return image_tensor;
}

static int libcompress_Main_pack(lua_State *L) {
    THByteTensor* image_tensor = luaT_checkudata(L, 1, "torch.ByteTensor");
    THByteStorage* packed_data = libcompress_pack_png_string(image_tensor);
    luaT_pushudata(L, packed_data, "torch.ByteStorage");
    return 1;
}


static int libcompress_Main_unpack(lua_State *L) {
    THByteStorage* packed_data = luaT_checkudata(L, 1, "torch.ByteStorage");
    THByteTensor* image_tensor = luaT_toudata(L, 2, "torch.ByteTensor");
    THByteTensor* result_tensor = NULL;
    THLongStorage* tensor_dimensions = NULL;

    if(image_tensor == NULL) {
        if((tensor_dimensions = luaT_toudata(L, 2, "torch.LongStorage"))) {
            result_tensor = THByteTensor_newWithSize(tensor_dimensions, NULL);
        } else {
            luaL_error(L, "expected arguments: ByteStorage ( LongStorage | ByteTensor )");
        }
    } else {
        result_tensor = THByteTensor_newWithTensor(image_tensor);
    }

    /* Push the result Tensor on to the lua stack first, so if the unpack fails, the tensor doesn't leak. */
    luaT_pushudata(L, result_tensor, "torch.ByteTensor");
    libcompress_unpack_png_string(packed_data, result_tensor);
    return 1;
}


static const luaL_reg libcompress__Main__[] =
{
    {"compress", libcompress_Main_pack},
    {"decompress", libcompress_Main_unpack},
    {NULL, NULL}
};

DLL_EXPORT int luaopen_libcompress(lua_State *L)
{
    luaL_register(L, "libcompress", libcompress__Main__);
    return 1;
}
