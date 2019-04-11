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

#ifndef imgui_gl_wrapper_hpp
#define imgui_gl_wrapper_hpp

#include <memory>
#include <vector>
#include <map>
#include <string>

#include "linalg_util.hpp"
#include "util.hpp"

// Implicit casts for linalg types
#define IM_VEC2_CLASS_EXTRA                                             \
ImVec2(const float2 & f) { x = f.x; y = f.y; }                          \
operator float2() const { return float2(x,y); }                         \
ImVec2(const int2 & f) { x = (float) f.x; y = (float) f.y; }            \
operator int2() const { return int2( (int) x, (int) y); }

#define IM_VEC4_CLASS_EXTRA                                             \
ImVec4(const float4 & f) { x = f.x; y = f.y; z = f.z; w = f.w; }        \
operator float4() const { return float4(x,y,z,w); }

#include "imgui/imgui.h"

struct GlTexture2D;

struct GLFWwindow;
namespace gui
{
    struct imgui_app_state : public singleton<imgui_app_state>
    {
        GLFWwindow * window;
        double       Time = 0.0f;
        bool         MousePressed[3] = { false, false, false };
        float        MouseWheel = 0.0f;
        uint32_t     FontTexture = 0;
        int          ShaderHandle = 0, VertHandle = 0, FragHandle = 0;
        int          AttribLocationTex = 0, AttribLocationProjMtx = 0;
        int          AttribLocationPosition = 0, AttribLocationUV = 0, AttribLocationColor = 0;
        unsigned int VboHandle = 0, VaoHandle = 0, ElementsHandle = 0;
        friend class singleton<imgui_app_state>;
    };

    class imgui_manager
    {
        bool create_fonts_texture();
        bool create_render_objects();
        void destroy_render_objects();
    public:
        
        imgui_manager(GLFWwindow * win);
        ~imgui_manager();
        void update_input(const app_input_event & e);
        void begin_frame();
        void end_frame();
        bool capturedKeys[1024];
    };
    
    inline void make_light_theme()
    {
        ImGuiStyle & s = ImGui::GetStyle();

        s.WindowMinSize = ImVec2(160, 20);
        s.FramePadding = ImVec2(4, 2);
        s.ItemSpacing = ImVec2(6, 2);
        s.ItemInnerSpacing = ImVec2(6, 4);
        s.Alpha = 0.80f;
        s.WindowRounding = 0.0f;
        s.FrameRounding = 0.0f;
        s.IndentSpacing = 4.0f;
        s.ColumnsMinSpacing = 50.0f;
        s.GrabMinSize = 14.0f;
        s.GrabRounding = 4.0f;
        s.ScrollbarSize = 16.0f;
        s.ScrollbarRounding = 2.0f;

        s.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        s.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        s.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        s.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        s.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        s.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        s.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.11f, 0.05f, 0.40f);
        s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.11f, 0.05f, 0.90f);
        s.Colors[ImGuiCol_TitleBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        s.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
        s.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        s.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        s.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.53f);
        s.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
        s.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
        s.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        s.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.11f, 0.05f, 1.00f);
        s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.11f, 0.05f, 0.78f);
        s.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.90f, 0.11f, 0.05f, 1.00f);
        s.Colors[ImGuiCol_Button] = ImVec4(0.90f, 0.11f, 0.05f, 0.40f);
        s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.11f, 0.05f, 1.00f);
        s.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.25f, 1.00f);
        s.Colors[ImGuiCol_Header] = ImVec4(0.90f, 0.11f, 0.05f, 0.8f);
        s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.11f, 0.05f, 0.80f);
        s.Colors[ImGuiCol_HeaderActive] = ImVec4(0.90f, 0.11f, 0.05f, 1.00f);
        s.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        s.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.90f, 0.11f, 0.05f, 0.67f);
        s.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.90f, 0.11f, 0.05f, 0.95f);
        s.Colors[ImGuiCol_CloseButton] = ImVec4(0.60f, 0.60f, 0.60f, 0.50f);
        s.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.25f, 0.39f, 0.36f, 1.00f);
        s.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.25f, 0.39f, 0.36f, 1.00f);
        s.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        s.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        s.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        s.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        s.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.90f, 0.11f, 0.05f, 0.35f);
        s.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }

    inline void imgui_fixed_window_begin(const char * name, const ui_rect & r)
    {
        ImGui::SetNextWindowPos(r.min);
        ImGui::SetNextWindowSize(r.max - r.min);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
        bool result = ImGui::Begin(name, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored({ 1,1,0.5f,1 }, name);
        ImGui::Separator();
        assert(result);
    }

    inline void imgui_fixed_window_end()
    {
        ImGui::End();
        ImGui::PopStyleVar(2);
    }
}

#endif // imgui_gl_wrapper_hpp
