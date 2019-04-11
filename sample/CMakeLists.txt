cmake_minimum_required(VERSION 3.7)
PROJECT(gl-sculpting-kelvinlets)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(OpenGL REQUIRED)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../code")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")
set(SOURCES 
    "${SRC_DIR}/app.cpp" 
    "${SRC_DIR}/util.hpp"
    "${CORE_DIR}/deformation.h" 
    "${CORE_DIR}/glslmathforcpp.h" 
    "${CORE_DIR}/kelvinlets.h" 
    "${CORE_DIR}/nonelastic.h" 
    "${CORE_DIR}/odesolvers.h" 
)

set(EXTRA_SOURCES 
    "${SRC_DIR}/vr-hmd.hpp"
    "${SRC_DIR}/vr-hmd.cpp"
    "${SRC_DIR}/gl-api.hpp"
    "${SRC_DIR}/gl-geometry.hpp"
    "${SRC_DIR}/gl-imgui.hpp"
    "${SRC_DIR}/gl-imgui.cpp"
    "${SRC_DIR}/linalg.h"
    "${SRC_DIR}/linalg_util.hpp"
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# application
add_executable(${PROJECT_NAME} ${SOURCES} ${EXTRA_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${CORE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${THIRD_PARTY_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
source_group("src" FILES ${SOURCES})
source_group("extra" FILES ${EXTRA_SOURCES})

# glfw
set(GLFW_DIR "${THIRD_PARTY_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# glad loader for gl 3.3 core
set(GLAD_DIR "${THIRD_PARTY_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} "glad" "${CMAKE_DL_LIBS}")

# imgui
set(IMGUI_DIR "${THIRD_PARTY_DIR}/imgui")
add_library("imgui" "${IMGUI_DIR}/imgui_draw.cpp" "${IMGUI_DIR}/imgui.cpp")
target_include_directories("imgui" PRIVATE "${IMGUI_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}")
target_link_libraries(${PROJECT_NAME} "imgui" "${CMAKE_DL_LIBS}")

# openvr
set(OPENVR_DIR "${THIRD_PARTY_DIR}/openvr")
target_include_directories(${PROJECT_NAME} PUBLIC "${OPENVR_DIR}/include")
target_link_libraries(${PROJECT_NAME} "${OPENVR_DIR}/lib/x64/openvr_api.lib")

add_custom_target(CopyBinaries
    COMMAND ${CMAKE_COMMAND} -E copy ${OPENVR_DIR}/bin/x64/openvr_api.dll ${EXECUTABLE_OUTPUT_PATH}
)

# file(COPY ${DIR}/openvr_api.dll DESTINATION ${EXECUTABLE_OUTPUT_PATH})

set_target_properties(glfw PROPERTIES FOLDER "third-party")
set_target_properties(glad PROPERTIES FOLDER "third-party")
set_target_properties(imgui PROPERTIES FOLDER "third-party")