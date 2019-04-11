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

#ifndef vr_hmd_hpp
#define vr_hmd_hpp

#include "openvr/include/openvr.h"

#include "util.hpp"
#include "gl-api.hpp"

#include <vector>
#include <memory>

struct OpenVR_Controller
{
    Transform p;
    bool trigger = false;

    struct ControllerRenderData
    {
        gl_mesh mesh;
        gl_texture_2d tex;
        bool loaded = false;
    };

    std::shared_ptr<ControllerRenderData> renderData;
};

inline Transform make_pose(const vr::HmdMatrix34_t & m)
{
    return {
        make_rotation_quat_from_rotation_matrix({ { m.m[0][0], m.m[1][0], m.m[2][0] },{ m.m[0][1], m.m[1][1], m.m[2][1] },{ m.m[0][2], m.m[1][2], m.m[2][2] } }),
        { m.m[0][3], m.m[1][3], m.m[2][3] }
    };
}

class OpenVR_HMD
{
    vr::IVRSystem * hmd{ nullptr };
    vr::IVRRenderModels * renderModels{ nullptr };

    uint2 renderTargetSize;
    Transform hmdPose;

public:

    std::shared_ptr<OpenVR_Controller::ControllerRenderData> controllerRenderData;
    OpenVR_Controller controllers[2];

    Transform displayTransform;
    Transform cameraToHeadTransform;

    OpenVR_HMD();
    ~OpenVR_HMD();

    Transform get_hmd_pose() const { return hmdPose; }
    uint2 get_recommended_render_target_size() { return renderTargetSize; }
    float4x4 get_proj_matrix(vr::Hmd_Eye eye, float near_clip, float far_clip) { return transpose(reinterpret_cast<const float4x4 &>(hmd->GetProjectionMatrix(eye, near_clip, far_clip, vr::API_OpenGL))); }
    Transform get_eye_pose(vr::Hmd_Eye eye) { return get_hmd_pose() * make_pose(hmd->GetEyeToHeadTransform(eye)); }

    void update();
    void submit(const GLuint leftEye, const GLuint rightEye);

    vr::IVRSystem * get_ivrsystem() { return hmd; }
};

#endif

