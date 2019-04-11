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

#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <string>
#include <stdint.h>
#include <unordered_map>

#include "linalg_util.hpp"
#include "gl-api.hpp"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

using namespace linalg;
using namespace linalg::aliases;

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
    noncopyable(const noncopyable& r) = delete;
    noncopyable & operator = (const noncopyable& r) = delete;
};

template <typename T>
class singleton : public noncopyable
{
private:
    singleton(const singleton<T> &);
    singleton & operator = (const singleton<T> &);
protected:
    static T * single;
    singleton() = default;
    ~singleton() = default;
public:
    static T & get_instance() { if (!single) single = new T(); return *single; };
};

struct app_input_event
{
    enum Type { CURSOR, MOUSE, KEY, CHAR, SCROLL };

    GLFWwindow * window;
    linalg::aliases::int2 windowSize;

    Type type;
    int action;
    int mods;

    linalg::aliases::float2 cursor;
    bool drag = false;

    linalg::aliases::uint2 value; // button, key, codepoint, scrollX, scrollY

    bool is_down() const { return action != GLFW_RELEASE; }
    bool is_up() const { return action == GLFW_RELEASE; }
};

static app_input_event make_input_event(GLFWwindow * window, app_input_event::Type type, const linalg::aliases::float2 cursor, int action)
{
    static bool isDragging = false;

    app_input_event e;
    e.window = window;
    e.type = type;
    e.cursor = cursor;
    e.action = action;
    e.mods = 0;

    glfwGetWindowSize(window, &e.windowSize.x, &e.windowSize.y);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) | glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)) e.mods |= GLFW_MOD_SHIFT;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) | glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)) e.mods |= GLFW_MOD_CONTROL;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) | glfwGetKey(window, GLFW_KEY_RIGHT_ALT)) e.mods |= GLFW_MOD_ALT;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) | glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)) e.mods |= GLFW_MOD_SUPER;

    if (type == app_input_event::MOUSE)
    {
        if (e.is_down()) isDragging = true;
        else if (e.is_up()) isDragging = false;
    }
    e.drag = isDragging;

    return e;
}

struct simple_interactive_camera
{
    bool ml = 0, mr = 0, bf = 0, bl = 0, bb = 0, br = 0;
    bool up = 0;
    bool down = 0;
    float2 lastCursor;
    float yfov, near_clip, far_clip;
    float3 position;
    float pitch, yaw;

    float4 get_orientation() const { return qmul(rotation_quat(float3(0, 1, 0), yaw), rotation_quat(float3(1, 0, 0), pitch)); }
    float4x4 get_view_matrix() const { return mul(rotation_matrix(qconj(get_orientation())), translation_matrix(-position)); }
    float4x4 get_projection_matrix(const float aspect) const { return linalg::perspective_matrix(yfov, aspect, near_clip, far_clip); }
    float4x4 get_viewproj_matrix(const float aspect) const { return mul(get_projection_matrix(aspect), get_view_matrix()); }

    void update_input(const app_input_event & e)
    {
        if (e.type == app_input_event::KEY)
        {
            if (e.value[0] == GLFW_KEY_W) bf = e.is_down();
            if (e.value[0] == GLFW_KEY_A) bl = e.is_down();
            if (e.value[0] == GLFW_KEY_S) bb = e.is_down();
            if (e.value[0] == GLFW_KEY_D) br = e.is_down();            
            if (e.value[0] == GLFW_KEY_E) up = e.is_down();
            if (e.value[0] == GLFW_KEY_Q) down = e.is_down();
        }
        else if (e.type == app_input_event::MOUSE)
        {
            if (e.value[0] == GLFW_MOUSE_BUTTON_LEFT) ml = e.is_down();
            if (e.value[0] == GLFW_MOUSE_BUTTON_RIGHT) mr = e.is_down();
        }
        else if (e.type == app_input_event::CURSOR)
        {
            auto deltaCursorMotion = e.cursor - lastCursor;
            if (mr)
            {
                yaw -= deltaCursorMotion.x * 0.005f;
                pitch -= deltaCursorMotion.y * 0.005f;
            }
            lastCursor = float2(e.cursor.x, e.cursor.y);
        }
    }

    void update(const float timestep)
    {
        if (mr)
        {
            linalg::aliases::float3 move;
            if (bf) move -= qzdir(get_orientation());
            if (bl) move -= qxdir(get_orientation());
            if (bb) move += qzdir(get_orientation());
            if (br) move += qxdir(get_orientation());            
            if (up) move += qydir(get_orientation());
            if (down) move -= qydir(get_orientation());
            if (length2(move) > 0) position += normalize(move) * (timestep * 10);
        }
    }
};

struct simple_texture_view
{
    gl_shader shader;
    gl_mesh fullscreen_quad_ndc;

    simple_texture_view()
    {
        static const char s_textureVert[] = R"(#version 330
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec2 uvs;
            uniform mat4 u_mvp = mat4(1);
            out vec2 texCoord;
            void main()
            {
                texCoord = vec2(uvs.x, uvs.y);
                gl_Position = u_mvp * vec4(position.xy, 0.0, 1.0);
    
            }
        )";

        static const char s_textureFrag[] = R"(#version 330
            uniform sampler2D s_texture;
            in vec2 texCoord;
            out vec4 f_color;
            void main()
            {
                vec4 sample = texture(s_texture, texCoord);
                f_color = vec4(sample.rgb, 1.0);
            }
        )";

        shader = gl_shader(s_textureVert, s_textureFrag);

        struct Vertex { linalg::aliases::float3 position; linalg::aliases::float2 texcoord; };
        const linalg::aliases::float3 verts[6] = { { -1.0f, -1.0f, 0.0f },{ 1.0f, -1.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{ -1.0f, 1.0f, 0.0f },{ 1.0f, -1.0f, 0.0f },{ 1.0f, 1.0f, 0.0f } };
        const linalg::aliases::float2 texcoords[6] = { { 0, 0 },{ 1, 0 },{ 0, 1 },{ 0, 1 },{ 1, 0 },{ 1, 1 } };
        const linalg::aliases::uint3 faces[2] = { { 0, 1, 2 },{ 3, 4, 5 } };
        std::vector<Vertex> vertices;
        for (int i = 0; i < 6; ++i) vertices.push_back({ verts[i], texcoords[i] });

        fullscreen_quad_ndc.set_vertices(vertices, GL_STATIC_DRAW);
        fullscreen_quad_ndc.set_attribute(0, &Vertex::position);
        fullscreen_quad_ndc.set_attribute(1, &Vertex::texcoord);
        fullscreen_quad_ndc.set_elements(faces, GL_STATIC_DRAW);
    }

    void draw(const GLuint texture_handle)
    {
        const GLboolean wasDepthTestingEnabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);
        shader.bind();
        shader.texture("s_texture", 0, texture_handle, GL_TEXTURE_2D);
        fullscreen_quad_ndc.draw_elements();
        shader.unbind();
        if (wasDepthTestingEnabled) glEnable(GL_DEPTH_TEST);
    }
};

class window
{
    GLFWwindow * win;

public:

    std::function<void(unsigned int codepoint)> on_char;
    std::function<void(int key, int action, int mods)> on_key;
    std::function<void(int button, int action, int mods)> on_mouse_button;
    std::function<void(float2 pos)> on_cursor_pos;
    std::function<void(int numFiles, const char ** paths)> on_drop;
    std::function<void(app_input_event e)> on_input;

    window(int width, int height, const char * title)
    {
        if (glfwInit() == GL_FALSE)throw std::runtime_error("glfwInit() failed");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        win = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (win == nullptr) throw std::runtime_error("glfwCreateWindow() failed");

        glfwMakeContextCurrent(win);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("gladLoadGLLoader(...) failed");
        }

        std::cout << "GL_VERSION =  " << (char *)glGetString(GL_VERSION) << std::endl;
        std::cout << "GL_VENDOR =   " << (char *)glGetString(GL_VENDOR) << std::endl;
        std::cout << "GL_RENDERER = " << (char *)glGetString(GL_RENDERER) << std::endl;

        glfwSetWindowUserPointer(win, this);

        glfwSetCharCallback(win, [](GLFWwindow * win, unsigned int codepoint) {
            auto w = (window *)glfwGetWindowUserPointer(win); if (w->on_char) w->on_char(codepoint);
        });

        glfwSetKeyCallback(win, [](GLFWwindow * win, int key, int, int action, int mods) {
            auto w = (window *)glfwGetWindowUserPointer(win); if (w->on_key) w->on_key(key, action, mods);
        });

        glfwSetMouseButtonCallback(win, [](GLFWwindow * win, int button, int action, int mods) {
            auto w = (window *)glfwGetWindowUserPointer(win); if (w->on_mouse_button) w->on_mouse_button(button, action, mods);
        });

        glfwSetCursorPosCallback(win, [](GLFWwindow * win, double xpos, double ypos) {
            auto w = (window *)glfwGetWindowUserPointer(win); if (w->on_cursor_pos) w->on_cursor_pos(float2(double2(xpos, ypos)));
        });

        glfwSetDropCallback(win, [](GLFWwindow * win, int numFiles, const char ** paths) {
            auto w = (window *)glfwGetWindowUserPointer(win); if (w->on_drop) w->on_drop(numFiles, paths);
        });

        glfwSetWindowUserPointer(win, this);
    }

    ~window()
    {
        glfwMakeContextCurrent(win);
        glfwDestroyWindow(win);
        glfwTerminate();
    }

    window(const window &) = delete;
    window(window &&) = delete;
    window & operator = (const window &) = delete;
    window & operator = (window &&) = delete;

    GLFWwindow * get_glfw_window_handle() { return win; };
    bool should_close() const { return !!glfwWindowShouldClose(win); }
    int get_window_attrib(int attrib) const { return glfwGetWindowAttrib(win, attrib); }
    int2 get_window_size() const { int2 size; glfwGetWindowSize(win, &size.x, &size.y); return size; }
    void set_window_size(int2 newSize) { glfwSetWindowSize(win, newSize.x, newSize.y); }
    int2 get_framebuffer_size() const { int2 size; glfwGetFramebufferSize(win, &size.x, &size.y); return size; }
    float2 get_cursor_pos() const { double2 pos; glfwGetCursorPos(win, &pos.x, &pos.y); return float2(pos); }

    void swap_buffers() { glfwSwapBuffers(win); }
    void close() { glfwSetWindowShouldClose(win, 1); }
};
