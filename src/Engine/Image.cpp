#include "Image.h"
#include <GL/glew.h>
#include "Misc.h"
#include <vector>
#include <memory>
#include <iostream>
#include <iterator>
#include <exception>
#include <stdexcept>

#include <FreeImage.h>
#include <cstdio>

namespace engine {

using std::string;
using std::runtime_error;

template<typename T, typename Deleter>
inline std::unique_ptr<T, Deleter> make_unique_raw(T* t, Deleter&& d) {
    return std::unique_ptr<T, Deleter>(t, std::forward<Deleter>(d));
}

Image Image::_internalLoad(std::vector<char>&& vd, bool srgb)
{
    Image temporary;
    
    namespace texture_desc = gldr::texture_desc;

    // FIXME
    auto internalFormat = texture_desc::InternalFormat::RGBA;
    if (srgb)
        internalFormat = texture_desc::InternalFormat::SRGBA;
        
    auto format = texture_desc::Format::BGRA;
    GLuint level = 0;
    
    auto fm = make_unique_raw(FreeImage_OpenMemory((BYTE*)(vd.data()), vd.size()), &FreeImage_CloseMemory);

    //check the file signature and deduce its format
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fm.get(), vd.size());

    //if still unknown, try to guess the file format from the file extension
    //if(fif == FIF_UNKNOWN) 
    //    fif = FreeImage_GetFIFFromFilename(filename);
    //if still unkown, return failure
    if(fif == FIF_UNKNOWN)
        throw runtime_error("Failed to deduce image filetype");

    //check that the plugin has reading capabilities and load the file
    if(!FreeImage_FIFSupportsReading(fif))
        throw runtime_error("This image type is not supported");

    //auto dib = std::unique_ptr<FIBITMAP, FreeImageBitmapDeleter>(
    //    FreeImage_LoadFromMemory(fif, fm.get()), FreeImageBitmapDeleter());

    auto dib = make_unique_raw(FreeImage_LoadFromMemory(fif, fm.get()), &FreeImage_Unload);

    //    dib = FreeImage_Load(fif, path);

    //if the image failed to load, return failure
    if(!dib)
        throw runtime_error("Image failed to load");

    FreeImage_FlipVertical(dib.get()); 
    dib.reset(FreeImage_ConvertTo32Bits(dib.get()));

    //raw pointer to the image data
    BYTE* bits = FreeImage_GetBits(dib.get());
    //get the image width and height
    unsigned width = FreeImage_GetWidth(dib.get());
    unsigned height = FreeImage_GetHeight(dib.get());
    //if this somehow one of these failed (they shouldn't), return failure
    if((bits == 0) || (width == 0) || (height == 0))
        throw runtime_error("Image checking failed");

    //bind to the new texture ID
    temporary.bindToUnit(0);

    temporary.imageData(width, height, format, internalFormat, texture_desc::DataType::UnsignedByte, bits);
    //store the texture data for OpenGL use
    
    //return success
    return std::move(temporary);
}

int Image::GetWidth()
{
    int size;
    bindToUnit(0);
    gl::GetTexLevelParameteriv(gl::TEXTURE_2D, 0, gl::TEXTURE_WIDTH, &size);
    return size;
}


} // namespace engine
