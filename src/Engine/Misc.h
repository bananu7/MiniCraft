// misc.h
#pragma once
#include <list>

namespace engine {

const long double PI            = 3.1415926535897932384626433;
const float DESIRED_DELTA        = 0.0166667f;
const long double DEGREE_TO_RAD    = 0.01745329251994329576923690;//768489
const long double RAD_TO_DEGREE    = 57.295779513082320876798154;//814105

// first - iterator naszej Entity
// second - wskaznik na kafel, na ktorym sie znajduje
char* TextFileRead (const char* path);

//---------------------------------------------------------------------
int Rnd (float f);
float DotPart (float f);
float Average (float a, float b);
int Random (int range_min, int range_max);
float Random ();

template<class T>
inline T Sqre(const T A)
{
    return A*A;
}

/// Clampuje value do zakresu <a, b>
template<typename T>
inline T Range(T value, T a, T b)
{
    if (value<a)
        value=a;
    if (value>b)
        value=b;
    return value;
}

// From wiki: fast inverse sqrt
inline float InvSqrt(const float number)
{
        long i;
        float x2, y;
        const float threehalfs = 1.5F;
 
        x2 = number * 0.5F;
        y  = number;
        i  = * ( long * ) &y;                       // evil floating point bit level hacking [sic]
        i  = 0x5f3759df - ( i >> 1 );               // what the fuck? [sic]
        y  = * ( float * ) &i;
        y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//    y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
 
        return y;
}

float odl_od1_do2 (float X1, float Y1, float X2, float Y2);
float kat_od1_do2 (float X1, float Y1, float X2, float Y2);

} // namespace engine
