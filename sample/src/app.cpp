#include "util.hpp"
#include "gl-imgui.hpp"
#include "gl-geometry.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "vr-hmd.hpp"

#include "deformation.h"
#include "glslmathforcpp.h"
#include "kelvinlets.h"
#include "nonelastic.h"
#include "odesolvers.h"

std::unique_ptr<window> win;
std::unique_ptr<gui::imgui_manager> imgui;

int main(int argc, char * argv[])
{
    try { win.reset(new window(1280, 720, "gfx-app")); }
    catch (const std::exception & e) { std::cout << "Caught GLFW window exception: " << e.what() << std::endl; }
    
    imgui.reset(new gui::imgui_manager(win->get_glfw_window_handle()));
    gui::make_light_theme();

    simple_interactive_camera cam = {};
    cam.yfov = 1.33f;
    cam.near_clip = 0.001f;
    cam.far_clip = 256.0f;
    cam.position = { 0, 2.f, 5 };

    int2 windowSize = win->get_window_size();

    win->on_char = [&](int codepoint)
    {
        auto e = make_input_event(win->get_glfw_window_handle(), app_input_event::CHAR, win->get_cursor_pos(), 0);
        e.value[0] = codepoint;
        if (win->on_input) win->on_input(e);
    };

    win->on_key = [&](int key, int action, int mods)
    {
        auto e = make_input_event(win->get_glfw_window_handle(), app_input_event::KEY, win->get_cursor_pos(), action);
        e.value[0] = key;
        if (win->on_input) win->on_input(e);
    };

    win->on_mouse_button = [&](int button, int action, int mods)
    {
        auto e = make_input_event(win->get_glfw_window_handle(), app_input_event::MOUSE, win->get_cursor_pos(), action);
        e.value[0] = button;
        if (win->on_input) win->on_input(e);
    };

    win->on_cursor_pos = [&](linalg::aliases::float2 position)
    {
        auto e = make_input_event(win->get_glfw_window_handle(), app_input_event::CURSOR, position, 0);
        if (win->on_input) win->on_input(e);
    };

    float2 lastCursor;
    win->on_input = [&](const app_input_event & event)
    {
        imgui->update_input(event);
        cam.update_input(event);

        if (event.type == app_input_event::KEY)
        {
            if (event.value[0] == GLFW_KEY_ESCAPE) win->close();
        }
    };

    int width, height;
    glfwGetWindowSize(win->get_glfw_window_handle(), &width, &height);
    
    auto t0 = std::chrono::high_resolution_clock::now();
    while (!win->should_close())
    {
        glfwMakeContextCurrent(win->get_glfw_window_handle());
        glfwPollEvents();

        auto t1 = std::chrono::high_resolution_clock::now();
        float timestep = std::chrono::duration<float>(t1 - t0).count();
        t0 = t1;

        cam.update(timestep);

        glViewport(0, 0, width, height);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.f, 1.f, 1.f, 1.f);

        const float3 eye = cam.position;
        const float4x4 projectionMatrix = cam.get_projection_matrix((float)windowSize.x / (float)windowSize.y);
        const float4x4 viewMatrix = cam.get_view_matrix();
        const float4x4 viewProjMatrix = mul(projectionMatrix, viewMatrix);

        imgui->begin_frame();
        gui::imgui_fixed_window_begin("gl-sculpting-kelvinlets", { { 0, 0 },{ 300, windowSize.y } });
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        gui::imgui_fixed_window_end();
        imgui->end_frame();

        win->swap_buffers();
    }

    imgui.reset();
    return EXIT_SUCCESS;
}
