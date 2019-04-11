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

#include "vr-hmd.hpp"

#include <iostream>
#include <string>

#include "util.hpp"

std::string get_tracked_device_string(vr::IVRSystem * pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
    uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
    if (unRequiredBufferLen == 0) return "";
    std::vector<char> pchBuffer(unRequiredBufferLen);
    unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer.data(), unRequiredBufferLen, peError);
    std::string result = { pchBuffer.begin(), pchBuffer.end() };
    return result;
}

///////////////////////////////////
//   OpenVR HMD Implementation   //
///////////////////////////////////

OpenVR_HMD::OpenVR_HMD()
{
    vr::EVRInitError eError = vr::VRInitError_None;
    hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);
    if (eError != vr::VRInitError_None) throw std::runtime_error("Unable to init VR runtime: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)));

    std::cout << "VR Driver:  " << get_tracked_device_string(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) << std::endl;
    std::cout << "VR Display: " << get_tracked_device_string(hmd, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String) << std::endl;

    controllerRenderData = std::make_shared<OpenVR_Controller::ControllerRenderData>();
    controllers[0].renderData = controllerRenderData;
    controllers[1].renderData = controllerRenderData;

    renderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
    if (!renderModels)
    {
        vr::VR_Shutdown();
        throw std::runtime_error("Unable to get render model interface: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)));
    }

    struct Vertex { float3 position; };

    {
        vr::RenderModel_t * model = nullptr;
        vr::RenderModel_TextureMap_t * texture = nullptr;

        while (true)
        {
            // see VREvent_TrackedDeviceActivated below for the proper way of doing this
            renderModels->LoadRenderModel_Async("vr_controller_vive_1_5", &model);
            if (model) renderModels->LoadTexture_Async(model->diffuseTextureId, &texture);
            if (model && texture) break;
        }

        std::vector<Vertex> vertices;
        std::vector<uint3> faces;

        for (uint32_t v = 0; v < model->unVertexCount; v++)
        {
            const vr::RenderModel_Vertex_t vertex = model->rVertexData[v];
            vertices.push_back({float3(vertex.vPosition.v[0], vertex.vPosition.v[1], vertex.vPosition.v[2])});
        }

        for (uint32_t f = 0; f < model->unTriangleCount * 3; f += 3)
        {
            faces.push_back({ model->rIndexData[f], model->rIndexData[f + 1] , model->rIndexData[f + 2] });
        }

        glTextureImage2DEXT(controllerRenderData->tex, GL_TEXTURE_2D, 0, GL_RGBA, texture->unWidth, texture->unHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->rubTextureMapData);
        glGenerateTextureMipmapEXT(controllerRenderData->tex, GL_TEXTURE_2D);
        glTextureParameteriEXT(controllerRenderData->tex, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteriEXT(controllerRenderData->tex, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        renderModels->FreeTexture(texture);
        renderModels->FreeRenderModel(model);

        controllerRenderData->mesh.set_vertices(vertices, GL_STATIC_DRAW);
        controllerRenderData->mesh.set_attribute(0, &Vertex::position);
        controllerRenderData->mesh.set_elements(faces, GL_STATIC_DRAW);

        controllerRenderData->loaded = true;
    }

    hmd->GetRecommendedRenderTargetSize(&renderTargetSize.x, &renderTargetSize.y);

    vr::ETrackedPropertyError err;
    displayTransform = make_pose(hmd->GetMatrix34TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_StatusDisplayTransform_Matrix34, &err));
    cameraToHeadTransform = make_pose(hmd->GetMatrix34TrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_CameraToHeadTransform_Matrix34, &err));

    // Setup the compositor
    if (!vr::VRCompositor())
    {
        throw std::runtime_error("could not initialize VRCompositor");
    }
}

OpenVR_HMD::~OpenVR_HMD()
{
    if (hmd) vr::VR_Shutdown();
}

void OpenVR_HMD::update()
{
    // Handle events
    vr::VREvent_t event;
    while (hmd->PollNextEvent(&event, sizeof(event)))
    {
        switch (event.eventType)
        {
        case vr::VREvent_TrackedDeviceDeactivated: std::cout << "Device " << event.trackedDeviceIndex << " detached." << std::endl; break;
        case vr::VREvent_TrackedDeviceUpdated: std::cout << "Device " << event.trackedDeviceIndex << " updated." << std::endl; break;
        }
    }

    // Get HMD pose
    std::array<vr::TrackedDevicePose_t, 16> poses;
    vr::VRCompositor()->WaitGetPoses(poses.data(), static_cast<uint32_t>(poses.size()), nullptr, 0);

    for (vr::TrackedDeviceIndex_t i = 0; i < poses.size(); ++i)
    {
        if (!poses[i].bPoseIsValid) continue;

        switch (hmd->GetTrackedDeviceClass(i))
        {
            case vr::TrackedDeviceClass_HMD:
            {
                hmdPose = make_pose(poses[i].mDeviceToAbsoluteTracking); 
                break;
            }
            case vr::TrackedDeviceClass_Controller:
            {
                vr::VRControllerState_t controllerState = vr::VRControllerState_t();
                switch (hmd->GetControllerRoleForTrackedDeviceIndex(i))
                {
                    case vr::TrackedControllerRole_LeftHand:
                    {

                        if (hmd->GetControllerState(i, &controllerState))
                        {
                            controllers[0].p = make_pose(poses[i].mDeviceToAbsoluteTracking);
                            controllers[0].trigger = !!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));
                        }

                        break;
                    }
                    case vr::TrackedControllerRole_RightHand:
                    {
                        if (hmd->GetControllerState(i, &controllerState))
                        {
                            controllers[1].p = make_pose(poses[i].mDeviceToAbsoluteTracking);
                            controllers[1].trigger = !!(controllerState.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

void OpenVR_HMD::submit(const GLuint leftEye, const GLuint rightEye)
{
    const vr::Texture_t leftTex = { (void*)(intptr_t) leftEye, vr::API_OpenGL, vr::ColorSpace_Gamma };
    vr::VRCompositor()->Submit(vr::Eye_Left, &leftTex);

    const vr::Texture_t rightTex = { (void*)(intptr_t) rightEye, vr::API_OpenGL, vr::ColorSpace_Gamma };
    vr::VRCompositor()->Submit(vr::Eye_Right, &rightTex);

    glFlush();
}