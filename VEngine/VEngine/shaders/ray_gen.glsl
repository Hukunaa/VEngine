#version 460
#extension GL_NV_ray_tracing : require

layout(set = 0, binding = 0) uniform accelerationStructureNV Scene;
layout(set = 0, binding = 1, rgba8) uniform image2D ResultImage;

layout(location = 0) rayPayloadNV vec3 ResultColor;

void main() {
    const vec2 uv = vec2(gl_LaunchIDNV.xy) / vec2(gl_LaunchSizeNV.xy - 1);

    const vec3 origin = vec3(uv.x, 1.0f - uv.y, -1.0f);
    const vec3 direction = vec3(0.0f, 0.0f, 1.0f);

    const uint rayFlags = gl_RayFlagsNoneNV;
    const uint cullMask = 0xFF;
    const uint sbtRecordOffset = 0;
    const uint sbtRecordStride = 0;
    const uint missIndex = 0;
    const float tmin = 0.0f;
    const float tmax = 10.0f;
    const int payloadLocation = 0;

    traceNV(Scene,
             rayFlags,
             cullMask,
             sbtRecordOffset,
             sbtRecordStride,
             missIndex,
             origin,
             tmin,
             direction,
             tmax,
             payloadLocation);

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(ResultColor, 1.0f));
}