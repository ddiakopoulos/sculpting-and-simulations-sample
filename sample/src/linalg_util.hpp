/* 
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <http://unlicense.org/>
 */

#ifndef linalg_util_hpp
#define linalg_util_hpp

#include "linalg.h"
#include <iostream>

using namespace linalg::aliases;

static const float4x4 Identity4x4 = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
static const float3x3 Identity3x3 = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
static const float2x2 Identity2x2 = {{1, 0}, {0, 1}};
    
static const float4x4 Zero4x4 = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const float3x3 Zero3x3 = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
static const float2x2 Zero2x2 = {{0, 0}, {0, 0}};

template<class T, int M> linalg::vec<T, M>  safe_normalize(const linalg::vec<T,M> & a)  { return a / std::max(T(1E-6), length(a)); }
template<class T, int N> linalg::mat<T,N,N> inv(const linalg::mat<T,N,N> & a)           { return inverse(a); }

template<class T> std::ostream & operator << (std::ostream & a, const linalg::vec<T,2> & b) { return a << '{' << b.x << ", " << b.y << '}'; }
template<class T> std::ostream & operator << (std::ostream & a, const linalg::vec<T,3> & b) { return a << '{' << b.x << ", " << b.y << ", " << b.z << '}'; }
template<class T> std::ostream & operator << (std::ostream & a, const linalg::vec<T,4> & b) { return a << '{' << b.x << ", " << b.y << ", " << b.z << ", " << b.w << '}'; }

template<class T, int N> std::ostream & operator << (std::ostream & a, const linalg::mat<T,2,N> & b) { return a << '\n' << b.row(0) << '\n' << b.row(1) << '\n'; }
template<class T, int N> std::ostream & operator << (std::ostream & a, const linalg::mat<T,3,N> & b) { return a << '\n' << b.row(0) << '\n' << b.row(1) << '\n' << b.row(2) << '\n'; }
template<class T, int N> std::ostream & operator << (std::ostream & a, const linalg::mat<T,4,N> & b) { return a << '\n' << b.row(0) << '\n' << b.row(1) << '\n' << b.row(2) << '\n' << b.row(3) << '\n'; }

template<typename T> inline T clamp(const T & val, const T & min, const T & max) { return std::min(std::max(val, min), max); }

struct ui_rect
{
    int2 min, max;
    bool contains(const int2 & p) const { return all(gequal(p, min)) && all(less(p, max)); }
};

inline float3 transform_coord(const float4x4 & transform, const float3 & coord)
{
    auto r = mul(transform, float4(coord, 1)); return (r.xyz() / r.w);
}

inline float3 transform_vector(const float4x4 & transform, const float3 & vector)
{
    return mul(transform, float4(vector, 0)).xyz();
}

inline float4 make_rotation_quat_axis_angle(const float3 & axis, float angle)
{
    return { axis * std::sin(angle / 2), std::cos(angle / 2) };
}

inline float4x4 make_translation_matrix(const float3 & translation)
{
    return{ { 1,0,0,0 },{ 0,1,0,0 },{ 0,0,1,0 },{ translation,1 } };
}

inline float4x4 make_rotation_matrix(const float4 & rotation)
{
    return { { qxdir(rotation),0 },{ qydir(rotation),0 },{ qzdir(rotation),0 },{ 0,0,0,1 } };
}

inline float4x4 make_rotation_matrix(const float3 & axis, float angle)
{
    return make_rotation_matrix(make_rotation_quat_axis_angle(axis, angle));
}

#endif // end linalg_util_hpp