#pragma once
#include "Resource.h"
#include <string>
#include "Misc.h"
#include <boost/range.hpp>
#include "Texture.hpp"

namespace engine {

class Image : public gldr::Texture<gldr::texture_desc::Type::Texture_2D> {
    static Image _internalLoad(std::vector<char>&& data, bool srgb);

public:
    template<class Range>
    static Image Load (Range&& range, bool loadAsSRGB = false) {
        return std::move(_internalLoad(std::vector<char>(boost::begin(range), boost::end(range)), loadAsSRGB));
    }

    void Unload (void) { }

    int GetWidth();

    Image() { };
    Image(Image&& other) : Texture(std::move(other)) { };
    Image& operator= (Image&& other) {
        Texture::operator=(std::move(other));
        return *this;
    }
};

} // namespace engine
