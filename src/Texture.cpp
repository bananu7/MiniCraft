#include "Texture.hpp"

template<>
void Texture<TextureType::Texture_1D>::imageData(
    unsigned width,
    unsigned height,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    TextureDataType dataType,
    const void* data)
{
    bind();
    glTexImage1D(GL_TEXTURE_1D,
        0,
        static_cast<GLint>(internalFormat),
        width,
        height,
        0,
        static_cast<GLenum>(format),
        data);
}

template<>
void Texture<TextureType::Texture_2D>::imageData(
    unsigned width,
    unsigned height,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    TextureDataType dataType,
    const void* data)
{
    bind();
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(internalFormat),
        width,
        height,
        0,
        static_cast<GLenum>(format),
        GL_UNSIGNED_BYTE,
        data);
}

template<>
void Texture<TextureType::Texture_3D>::imageData(
    unsigned width,
    unsigned height,
    TextureFormat format,
    TextureInternalFormat internalFormat,
    TextureDataType dataType,
    const void* data)
{
    bind();
    glTexImage3D(
        GL_TEXTURE_3D,
        0,
        static_cast<GLint>(internalFormat),
        width,
        height,
        0,
        0,
        static_cast<GLenum>(format),
        static_cast<GLenum>(dataType),
        data);
}