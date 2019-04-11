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

#ifndef gl_33_core_api_hpp
#define gl_33_core_api_hpp

#include <map>
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include "linalg.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace
{
    inline void gl_check_error(const char * file, int32_t line)
    {
#if defined(_DEBUG) || defined(DEBUG)
        GLint error = glGetError();
        if (error)
        {
            const char * errorStr = 0;
            switch (error)
            {
            case GL_INVALID_ENUM: errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: errorStr = "GL_OUT_OF_MEMORY"; break;
            default: errorStr = "unknown error"; break;
            }
            printf("GL error : %s, line %d : %s\n", file, line, errorStr);
            error = 0;
        }
#endif
    }

    inline size_t gl_size_bytes(GLenum type)
    {
        switch (type)
        {
        case GL_UNSIGNED_BYTE: return sizeof(uint8_t);
        case GL_UNSIGNED_SHORT: return sizeof(uint16_t);
        case GL_UNSIGNED_INT: return sizeof(uint32_t);
        default: throw std::logic_error("unknown element type"); break;
        }
    }
}

template<typename factory_t>
class gl_handle_object
{
    mutable GLuint handle {0};
public:
    gl_handle_object() = default;
    ~gl_handle_object() { if (handle) factory_t::destroy(handle); }
    gl_handle_object(const gl_handle_object & r) = delete;
    gl_handle_object & operator = (gl_handle_object && r) { std::swap(handle, r.handle); return *this; }
    gl_handle_object(gl_handle_object && r) { *this = std::move(r); }
    operator GLuint () const { if (!handle) factory_t::create(handle); return handle; }
    gl_handle_object & operator = (GLuint & other) { handle = other; return *this; }
    GLuint id() const { return handle; };
};

struct gl_buffer_factory { static void create(GLuint & x) { glGenBuffers(1, &x); }; static void destroy(GLuint x) { glDeleteBuffers(1, &x); }; };
struct gl_texture_factory { static void create(GLuint & x) { glGenTextures(1, &x); }; static void destroy(GLuint x) { glDeleteTextures(1, &x); }; };
struct gl_vao_factory { static void create(GLuint & x) { glGenVertexArrays(1, &x); }; static void destroy(GLuint x) { glDeleteVertexArrays(1, &x); }; };
struct gl_renderbuffer_factory { static void create(GLuint & x) { glGenRenderbuffers(1, &x); }; static void destroy(GLuint x) { glDeleteRenderbuffers(1, &x); }; };
struct gl_framebuffer_factory { static void create(GLuint & x) { glGenFramebuffers(1, &x); }; static void destroy(GLuint x) { glDeleteFramebuffers(1, &x); }; };

typedef gl_handle_object<gl_buffer_factory> gl_buffer_object;
typedef gl_handle_object<gl_texture_factory> gl_texture_object;
typedef gl_handle_object<gl_vao_factory> gl_vao_object;
typedef gl_handle_object<gl_renderbuffer_factory> gl_renderbuffer_object;
typedef gl_handle_object<gl_framebuffer_factory> gl_framebuffer_object;

struct gl_buffer : public gl_buffer_object
{
    GLsizeiptr size;
    gl_buffer() = default;
    void set_buffer_data(const GLsizeiptr s, const GLenum target, const GLvoid * data, const GLenum usage)
    { 
        this->size = s; 
        glBindBuffer(target, *this);
        glBufferData(target, size, data, usage);
        glBindBuffer(target, 0);
    }
    void set_buffer_data(const std::vector<GLubyte> & bytes, const GLenum target, const GLenum usage) {  set_buffer_data(bytes.size(), target, bytes.data(), usage); }

    void set_buffer_sub_data(const GLsizeiptr s, const GLenum target, const GLintptr offset, const GLvoid * data)
    { 
        glBindBuffer(target, *this);
        glBufferSubData(target, offset, size, data);
        glBindBuffer(target, 0);
    }
    void set_buffer_sub_data(const std::vector<GLubyte> & bytes, const GLenum target, const GLintptr offset, const GLenum usage) { set_buffer_sub_data(bytes.size(), target, offset, bytes.data()); }
};

struct gl_renderbuffer : public gl_renderbuffer_object
{
    float width{ 0 }, height{ 0 };
    gl_renderbuffer() = default;
    gl_renderbuffer(float width, float height) : width(width), height(height) {}
};

struct gl_framebuffer : public gl_framebuffer_object
{
    float width{ 0 }, height{ 0 };
    gl_framebuffer() = default;
    gl_framebuffer(float width, float height) : width(width), height(height) {}
    void check_complete() 
    { 
        glBindFramebuffer(GL_FRAMEBUFFER, *this);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) throw std::runtime_error("fbo incomplete"); 
    }
};

struct gl_texture_2d : public gl_texture_object
{
    gl_texture_2d() = default;
    void setup(GLsizei width, GLsizei height, GLenum internal_fmt, GLenum format, GLenum type, const GLvoid * pixels, bool createMipmap = false)
    {
        glBindTexture(GL_TEXTURE_2D, *this);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, width, height, 0, format, type, pixels);
        if (createMipmap) glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

static inline void compile_shader(GLuint program, GLenum type, const char * source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> buffer(length);
        glGetShaderInfoLog(shader, (GLsizei)buffer.size(), nullptr, buffer.data());
        glDeleteShader(shader);
        std::cerr << "GL Compile Error: " << buffer.data() << std::endl;
        std::cerr << "Source: " << source << std::endl;
        throw std::runtime_error("GLSL Compile Failure");
    }

    glAttachShader(program, shader);
    glDeleteShader(shader);
}

class gl_shader
{
    GLuint program;
    bool enabled = false;

protected:

    gl_shader(const gl_shader & r) = delete;
    gl_shader & operator = (const gl_shader & r) = delete;

public:

    gl_shader() : program() {}

    gl_shader(const GLuint type, const std::string & src)
    {
        program = glCreateProgram();

        ::compile_shader(program, type, src.c_str());

        glLinkProgram(program);

        GLint status, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status == GL_FALSE)
        {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<GLchar> buffer(length);
            glGetProgramInfoLog(program, (GLsizei)buffer.size(), nullptr, buffer.data());
            std::cerr << "GL Link Error: " << buffer.data() << std::endl;
            throw std::runtime_error("GLSL Link Failure");
        }
    }

    gl_shader(const std::string & vert, const std::string & frag)
    {
        program = glCreateProgram();

        ::compile_shader(program, GL_VERTEX_SHADER, vert.c_str());
        ::compile_shader(program, GL_FRAGMENT_SHADER, frag.c_str());

        glLinkProgram(program);

        GLint status, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status == GL_FALSE)
        {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<GLchar> buffer(length);
            glGetProgramInfoLog(program, (GLsizei)buffer.size(), nullptr, buffer.data());
            std::cerr << "GL Link Error: " << buffer.data() << std::endl;
            throw std::runtime_error("GLSL Link Failure");
        }
    }

    ~gl_shader() { if (program) glDeleteProgram(program); }

    gl_shader(gl_shader && r) : gl_shader() { *this = std::move(r); }

    GLuint handle() const { return program; }
    GLint get_uniform_location(const std::string & name) const { return glGetUniformLocation(program, name.c_str()); }

    gl_shader & operator = (gl_shader && r) { std::swap(program, r.program); return *this; }

    std::map<uint32_t, std::string> reflect()
    {
        std::map<uint32_t, std::string> locations;
        GLint count;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
        for (GLuint i = 0; i < static_cast<GLuint>(count); ++i)
        {
            char buffer[1024]; GLenum type; GLsizei length; GLint size, block_index;
            glGetActiveUniform(program, i, sizeof(buffer), &length, &size, &type, buffer);
            glGetActiveUniformsiv(program, 1, &i, GL_UNIFORM_BLOCK_INDEX, &block_index);
            if (block_index != -1) continue;
            GLint loc = glGetUniformLocation(program, buffer);
            locations[loc] = std::string(buffer);
        }
        return locations;
    }

    void uniform(const std::string & name, int scalar) const { glUniform1i(get_uniform_location(name), scalar); }
    void uniform(const std::string & name, float scalar) const { glUniform1f( get_uniform_location(name), scalar); }
    void uniform(const std::string & name, const linalg::aliases::float2 & vec) const { glUniform2fv(get_uniform_location(name), 1, &vec.x); }
    void uniform(const std::string & name, const linalg::aliases::float3 & vec) const { glUniform3fv(get_uniform_location(name), 1, &vec.x); }
    void uniform(const std::string & name, const linalg::aliases::float4 & vec) const { glUniform4fv(get_uniform_location(name), 1, &vec.x); }
    void uniform(const std::string & name, const linalg::aliases::float3x3 & mat) const { glUniformMatrix3fv(get_uniform_location(name), 1, GL_FALSE, &mat.x.x); }
    void uniform(const std::string & name, const linalg::aliases::float4x4 & mat) const { glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, &mat.x.x); }

    void uniform(const std::string & name, const int elements, const std::vector<int> & scalar) const { glUniform1iv(get_uniform_location(name), elements, scalar.data()); }
    void uniform(const std::string & name, const int elements, const std::vector<float> & scalar) const { glUniform1fv(get_uniform_location(name), elements, scalar.data()); }
    void uniform(const std::string & name, const int elements, const std::vector<linalg::aliases::float2> & vec) const { glUniform2fv(get_uniform_location(name), elements, &vec[0].x); }
    void uniform(const std::string & name, const int elements, const std::vector<linalg::aliases::float3> & vec) const { glUniform3fv(get_uniform_location(name), elements, &vec[0].x); }
    void uniform(const std::string & name, const int elements, const std::vector<linalg::aliases::float3x3> & mat) const { glUniformMatrix3fv(get_uniform_location(name), elements, GL_FALSE, &mat[0].x.x); }
    void uniform(const std::string & name, const int elements, const std::vector<linalg::aliases::float4x4> & mat) const { glUniformMatrix4fv(get_uniform_location(name), elements, GL_FALSE, &mat[0].x.x); }

    void texture(const char * name, int unit, GLuint tex, GLenum target) const 
    { 
        glUniform1i(get_uniform_location(name), unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(target, tex);
    }

    void bind() { if (program > 0) enabled = true; glUseProgram(program); }
    void unbind() { enabled = false; glUseProgram(0); }
};

class gl_mesh
{
    gl_vao_object vao;
    gl_buffer vertexBuffer, instanceBuffer, indexBuffer;

    GLenum drawMode = GL_TRIANGLES;
    GLenum indexType = 0;
    GLsizei vertexStride = 0, indexCount = 0;

public:

    gl_mesh() = default;
    gl_mesh(gl_mesh && r) { *this = std::move(r); }
    gl_mesh(const gl_mesh & r) = delete;
    gl_mesh & operator = (gl_mesh && r)
    {
        char buffer[sizeof(gl_mesh)];
        memcpy(buffer, this, sizeof(buffer));
        memcpy(this, &r, sizeof(buffer));
        memcpy(&r, buffer, sizeof(buffer));
        return *this;
    }
    gl_mesh & operator = (const gl_mesh & r) = delete;
    ~gl_mesh() {};

    void set_non_indexed(GLenum newMode)
    {
        drawMode = newMode;
        indexBuffer = {};
        indexType = 0;
        indexCount = 0;
    }

    void draw_elements(int instances = 0) const
    {
        if (vertexBuffer.size)
        {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            if (indexCount)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                if (instances) glDrawElementsInstanced(drawMode, indexCount, indexType, 0, instances);
                else glDrawElements(drawMode, indexCount, indexType, nullptr); // with nullptr, gl will use the data bound to the current array
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            else
            {
                if (instances) glDrawArraysInstanced(drawMode, 0, static_cast<GLsizei>(vertexBuffer.size / vertexStride), instances);
                else glDrawArrays(drawMode, 0, static_cast<GLsizei>(vertexBuffer.size / vertexStride));
            }
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    void set_vertex_data(GLsizeiptr size, const GLvoid * data, GLenum usage) { vertexBuffer.set_buffer_data(size, GL_ARRAY_BUFFER, data, usage); }
    gl_buffer & get_vertex_data_buffer() { return vertexBuffer; };

    void set_instance_data(GLsizeiptr size, const GLvoid * data, GLenum usage) { instanceBuffer.set_buffer_data(size, GL_ARRAY_BUFFER, data, usage); }

    void set_index_data(GLenum mode, GLenum type, GLsizei count, const GLvoid * data, GLenum usage)
    {
        size_t size = gl_size_bytes(type);
        indexBuffer.set_buffer_data(size * count, GL_ELEMENT_ARRAY_BUFFER, data, usage);
        drawMode = mode;
        indexType = type;
        indexCount = count;
    }
    gl_buffer & get_index_data_buffer() { return indexBuffer; };

    void set_attribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * offset)
    {
        vertexStride = stride;

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(index, size, type, normalized, stride, offset);
        glVertexAttribDivisor(index, 0); // FIXME for instancing
        glEnableVertexAttribArray(index);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void set_indices(GLenum mode, GLsizei count, const uint8_t * indices, GLenum usage) { set_index_data(mode, GL_UNSIGNED_BYTE, count, indices, usage); }
    void set_indices(GLenum mode, GLsizei count, const uint16_t * indices, GLenum usage) { set_index_data(mode, GL_UNSIGNED_SHORT, count, indices, usage); }
    void set_indices(GLenum mode, GLsizei count, const uint32_t * indices, GLenum usage) { set_index_data(mode, GL_UNSIGNED_INT, count, indices, usage); }

    template<class T> void set_vertices(size_t count, const T * vertices, GLenum usage) { set_vertex_data(count * sizeof(T), vertices, usage); }
    template<class T> void set_vertices(const std::vector<T> & vertices, GLenum usage) { set_vertices(vertices.size(), vertices.data(), usage); }
    template<class T, int N> void set_vertices(const T(&vertices)[N], GLenum usage) { set_vertices(N, vertices, usage); }

    template<class V> void set_attribute(GLuint index, float V::*field) { set_attribute(index, 1, GL_FLOAT, GL_FALSE, sizeof(V), &(((V*)0)->*field)); }
    template<class V, int N> void set_attribute(GLuint index, linalg::vec<float, N> V::*field) { set_attribute(index, N, GL_FLOAT, GL_FALSE, sizeof(V), &(((V*)0)->*field)); }

    template<class T> void set_elements(GLsizei count, const linalg::vec<T, 2> * elements, GLenum usage) { set_indices(GL_LINES, count * 2, &elements->x, usage); }
    template<class T> void set_elements(GLsizei count, const linalg::vec<T, 3> * elements, GLenum usage) { set_indices(GL_TRIANGLES, count * 3, &elements->x, usage); }
    template<class T> void set_elements(GLsizei count, const linalg::vec<T, 4> * elements, GLenum usage) { set_indices(GL_QUADS, count * 4, &elements->x, usage); }
    template<class T> void set_elements(const std::vector<T> & elements, GLenum usage) { set_elements((GLsizei)elements.size(), elements.data(), usage); }
    template<class T, int N> void set_elements(const T(&elements)[N], GLenum usage) { set_elements(N, elements, usage); }
};

#endif // end gl_33_core_api_hpp
