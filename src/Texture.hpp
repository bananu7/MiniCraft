#pragma once
#include <GL/glew.h>

enum class TextureType : GLenum {
    Texture_1D = GL_TEXTURE_1D,
    Texture_2D = GL_TEXTURE_2D,
    Texture_3D = GL_TEXTURE_3D
};

enum class FilteringDirection : GLenum {
    Minification = GL_TEXTURE_MIN_FILTER,
    Magnification = GL_TEXTURE_MAG_FILTER
};

enum class FilteringMode : GLint {
    Linear = GL_LINEAR,
    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
    LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
    Nearest = GL_NEAREST,
    NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST
};

enum class TextureInternalFormat : GLenum {
    RGBA = GL_RGBA,
    RGB = GL_RGB,
    SRGB = GL_SRGB8,
    SRGBA = GL_SRGB8_ALPHA8,
    Depth = GL_DEPTH_COMPONENT
};

enum class TextureFormat : GLenum {
    RGBA = GL_RGBA,
    RGB = GL_RGB,
    Depth = GL_DEPTH_COMPONENT
};

enum class TextureDataType : GLenum {
    Float = GL_FLOAT,
    UnsignedByte = GL_UNSIGNED_BYTE
};

template<TextureType type>
class Texture {
    GLuint id;

    void _generateId() {
        if (id == 0)
            glGenTextures(1, &id);
    }
public:
    void bind() {
        _generateId();
        glBindTexture(static_cast<GLenum>(type), id);
    }

    void bind(unsigned textureUnit) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        bind();
    }

    void setFiltering (FilteringDirection direction, FilteringMode mode) {
        bind();
        glTexParameteri(static_cast<GLenum>(type), static_cast<GLenum>(direction), static_cast<GLint>(mode));
    }

    void imageData(unsigned width, unsigned height, TextureFormat format, TextureInternalFormat internalFormat, TextureDataType dataType, const void* data);

    GLuint getId() const { return id; }

    Texture () : id(0) {
    }



    Texture(Texture&& other) {
        id = other.id;
        other.id = 0;
    }

    Texture& operator= (Texture&& other) {
        id = other.id;
        other.id = 0;
        return *this;
    }

    ~Texture() {
        if (id) glDeleteTextures(1, &id);
    }

private:
    Texture (Texture const&)/* = delete*/;
    Texture& operator= (Texture const&)/* = delete*/;
};
