#version 460
#extension GL_NV_ray_tracing : require

layout(set = 0, binding = 0) uniform accelerationStructureNV Scene;
layout(set = 0, binding = 1, rgba8) uniform image2D ResultImage;
layout(set = 0, binding = 2) uniform UniformData 
{
    mat4 viewInverse;
    mat4 projInverse;
} ubo;

layout(location = 0) rayPayloadNV vec3 ResultColor;

void main() {
    const vec2 pixelCenter = vec2(gl_LaunchIDNV.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeNV.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec4 origin = ubo.viewInverse * vec4(0,0,0,1);
	vec4 target = ubo.projInverse * vec4(d.x, d.y, 1, 1) ;
	vec4 direction = ubo.viewInverse*vec4(normalize(target.xyz), 0);

    vec3 forigin = vec3(origin.xyz);
    vec3 fdir = vec3(direction.xyz);

    const uint rayFlags = gl_RayFlagsNoneNV;
    const uint cullMask = 0xFF;
    const uint sbtRecordOffset = 0;
    const uint sbtRecordStride = 0;
    const uint missIndex = 0;
    const float tmin = 0.0f;
    const float tmax = 100.0f;
    const int payloadLocation = 0;

    traceNV(Scene,
             rayFlags,
             cullMask,
             sbtRecordOffset,
             sbtRecordStride,
             missIndex,
             forigin,
             tmin,
             fdir,
             tmax,
             payloadLocation);

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(ResultColor, 1.0f));
}