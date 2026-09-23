// Struct layouts and constants shared by C++ and GLSL.
#ifndef GPU_SHARED_H
#define GPU_SHARED_H

#ifdef __cplusplus

#include <cstdint>
#include <glm/glm.hpp>

#define GPU_NAMESPACE_BEGIN namespace gpu {
#define GPU_NAMESPACE_END }
#define GPU_CONST static constexpr

// Alias C++/GLM types to the matching GLSL types for compatibility
GPU_NAMESPACE_BEGIN
using uint = uint32_t;
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using uvec2 = glm::uvec2;
GPU_NAMESPACE_END

#else // GLSL

// Enable 64-bit integer types and scalar block layout (GLSL types
// aligned to their component size, matching C++'s data layout).
// The latter avoids standard GLSL padding, so a vec3 will align to 4 bytes.
// The former lets us use int64 in both GLSL and C++, which simplifies buffer
// device address management.
// 
// Note that scalar layout only applies to blocks declared with layout(scalar).
// #extension must precede declarations, so other shaders will need to include
// this header first.
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define GPU_NAMESPACE_BEGIN
#define GPU_NAMESPACE_END
#define GPU_CONST const

#endif

GPU_NAMESPACE_BEGIN

// Constants for descriptor set numbers and binding indices
GPU_CONST uint kRendererDescriptorSet = 0;
GPU_CONST uint kTlasBinding = 0;
GPU_CONST uint kAccumImageBinding = 1;
GPU_CONST uint kDisplayImageBinding = 2;

struct CameraParams
{
    vec3 position;
    vec3 view;
    vec3 right;
    vec3 up;
    vec2 pixelLength;
};

// Push constant limits are per-device, with 128 bytes guaranteed
// by Vulkan.
// Running total: 4 * vec3 (48) + 3 * uint (12) + vec2 (8) = 68.
struct PushConstants
{
    CameraParams camera;
    uint renderedFrameCount;
    uint maxDepth;
    uint flags;
};

GPU_NAMESPACE_END

#ifdef __cplusplus
static_assert(sizeof(gpu::PushConstants) == 68,
    "Cpp PushConstants size must match the GLSL scalar-layout block in shaders.");
#endif

#endif // GPU_SHARED_H
