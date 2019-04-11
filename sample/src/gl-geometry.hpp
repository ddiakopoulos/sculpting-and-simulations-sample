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

#pragma once

#include "gl-api.hpp"
#include "util.hpp"

#define GFX_PI            3.1415926535897931
#define GFX_HALF_PI       1.5707963267948966
#define GFX_QUARTER_PI    0.7853981633974483
#define GFX_TWO_PI        6.2831853071795862
#define GFX_TAU           GFX_TWO_PI
#define GFX_INV_PI        0.3183098861837907
#define GFX_INV_TWO_PI    0.1591549430918953
#define GFX_INV_HALF_PI   0.6366197723675813

struct geometry
{
    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float2> texCoords;
    std::vector<uint3> triangles;
};

inline geometry make_plane_geometry(float width, float height, uint32_t nw, uint32_t nh)
{
    geometry g;

    uint32_t indexOffset = 0;

    float rw = 1.f / width;
    float rh = 1.f / height;
    float ow = width / nw;
    float oh = height / nh;

    float ou = ow * rw;
    float ov = oh * rh;

    for (float w = -width / 2.0f; w < width / 2.0f; w += ow)
    {
        for (float h = -height / 2.0f; h < height / 2.0f; h += oh)
        {
            float u = (w + width / 2.0f) * rw;
            float v = (h + height / 2.0f) * rh;

            g.vertices.push_back({ w, h + oh, 0.f });
            g.texCoords.push_back({ u, v + ov });

            g.vertices.push_back({ w, h, 0.f });
            g.texCoords.push_back({ u, v });

            g.vertices.push_back({ w + ow, h, 0.f });
            g.texCoords.push_back({ u + ou, v });

            g.vertices.push_back({ w + ow, h + oh, 0.f });
            g.texCoords.push_back({ u + ou, v + ov });

            g.triangles.push_back({ indexOffset + 0, indexOffset + 1, indexOffset + 2 });
            g.triangles.push_back({ indexOffset + 0, indexOffset + 2, indexOffset + 3 });

            indexOffset += 4;
        }
    }

    return g;
}

inline geometry make_cube_geometry()
{
    geometry cube;

    const struct CubeVertex { float3 position, normal; float2 texCoord; } verts[] =
    {
        { { -1, -1, -1 },{ -1, 0, 0 },{ 0, 0 } },{ { -1, -1, +1 },{ -1, 0, 0 },{ 1, 0 } },{ { -1, +1, +1 },{ -1, 0, 0 },{ 1, 1 } },{ { -1, +1, -1 },{ -1, 0, 0 },{ 0, 1 } },
        { { +1, -1, +1 },{ +1, 0, 0 },{ 0, 0 } },{ { +1, -1, -1 },{ +1, 0, 0 },{ 1, 0 } },{ { +1, +1, -1 },{ +1, 0, 0 },{ 1, 1 } },{ { +1, +1, +1 },{ +1, 0, 0 },{ 0, 1 } },
        { { -1, -1, -1 },{ 0, -1, 0 },{ 0, 0 } },{ { +1, -1, -1 },{ 0, -1, 0 },{ 1, 0 } },{ { +1, -1, +1 },{ 0, -1, 0 },{ 1, 1 } },{ { -1, -1, +1 },{ 0, -1, 0 },{ 0, 1 } },
        { { +1, +1, -1 },{ 0, +1, 0 },{ 0, 0 } },{ { -1, +1, -1 },{ 0, +1, 0 },{ 1, 0 } },{ { -1, +1, +1 },{ 0, +1, 0 },{ 1, 1 } },{ { +1, +1, +1 },{ 0, +1, 0 },{ 0, 1 } },
        { { -1, -1, -1 },{ 0, 0, -1 },{ 0, 0 } },{ { -1, +1, -1 },{ 0, 0, -1 },{ 1, 0 } },{ { +1, +1, -1 },{ 0, 0, -1 },{ 1, 1 } },{ { +1, -1, -1 },{ 0, 0, -1 },{ 0, 1 } },
        { { -1, +1, +1 },{ 0, 0, +1 },{ 0, 0 } },{ { -1, -1, +1 },{ 0, 0, +1 },{ 1, 0 } },{ { +1, -1, +1 },{ 0, 0, +1 },{ 1, 1 } },{ { +1, +1, +1 },{ 0, 0, +1 },{ 0, 1 } },
    };

    std::vector<uint4> quads = { { 0, 1, 2, 3 },{ 4, 5, 6, 7 },{ 8, 9, 10, 11 },{ 12, 13, 14, 15 },{ 16, 17, 18, 19 },{ 20, 21, 22, 23 } };

    for (auto & q : quads)
    {
        cube.triangles.push_back({ q.x,q.y,q.z });
        cube.triangles.push_back({ q.x,q.z,q.w });
    }

    for (int i = 0; i < 24; ++i)
    {
        cube.vertices.push_back(verts[i].position);
        cube.normals.push_back(verts[i].normal);
        cube.texCoords.push_back(verts[i].texCoord);
    }

    return cube;
}

inline geometry make_fullscreen_quad_ndc_geometry()
{
    geometry g;
    g.vertices = { { -1.0f, -1.0f, 0.0f },{ 1.0f, -1.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{ 1.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } };
    g.texCoords = { { 0, 0 },{ 1, 0 },{ 0, 1 },{ 0, 1 },{ 1, 0 },{ 1, 1 } };
    g.triangles = { { 0, 1, 2 },{ 3, 4, 5 } };
    return g;
}

inline gl_mesh make_mesh_from_geometry(const geometry & geometry, const GLenum usage = GL_STATIC_DRAW)
{
    gl_mesh m;

    int vertexOffset = 0;
    int normalOffset = 0;
    int texOffset = 0;

    int components = 3;

    if (geometry.normals.size() != 0) { normalOffset = components; components += 3; }
    if (geometry.texCoords.size() != 0) { texOffset = components; components += 2; }

    std::vector<float> buffer;
    buffer.reserve(geometry.vertices.size() * components);

    for (size_t i = 0; i < geometry.vertices.size(); ++i)
    {
        buffer.push_back(geometry.vertices[i].x);
        buffer.push_back(geometry.vertices[i].y);
        buffer.push_back(geometry.vertices[i].z);

        if (normalOffset)
        {
            buffer.push_back(geometry.normals[i].x);
            buffer.push_back(geometry.normals[i].y);
            buffer.push_back(geometry.normals[i].z);
        }

        if (texOffset)
        {
            buffer.push_back(geometry.texCoords[i].x);
            buffer.push_back(geometry.texCoords[i].y);
        }
    }

    m.set_vertex_data(buffer.size() * sizeof(float), buffer.data(), usage);
    m.set_attribute(0, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), ((float*)0) + vertexOffset);
    if (normalOffset) m.set_attribute(1, 3, GL_FLOAT, GL_FALSE, components * sizeof(float), ((float*)0) + normalOffset);
    if (texOffset) m.set_attribute(2, 2, GL_FLOAT, GL_FALSE, components * sizeof(float), ((float*)0) + texOffset);
    if (geometry.triangles.size() > 0)m.set_elements(geometry.triangles, usage);

    return m;
}
