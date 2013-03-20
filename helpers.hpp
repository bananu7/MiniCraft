#pragma once

#include <boost/range/istream_range.hpp>
// istreambuf version of boost::istream_range; required for binary data
//FIXME traits support and possibly char<->uchar
template<class Elem> inline
boost::iterator_range<std::istreambuf_iterator<Elem> >
istreambuf_range(std::basic_istream<Elem>& in)
{
    return boost::iterator_range<std::istreambuf_iterator<Elem> >(
        std::istreambuf_iterator<Elem>(in),
        std::istreambuf_iterator<Elem>());
}
