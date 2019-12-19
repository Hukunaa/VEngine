#version 460
#extension GL_NV_ray_tracing : require

layout(set = 0, binding = 0) uniform accelerationStructureNV Scene;
layout(set = 0, binding = 1, rgba8) uniform image2D ResultImage;
layout(set = 0, binding = 2) uniform CamData 
{
    mat4 viewInverse;
    mat4 projInverse;
} ubo;

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    float reflectiveness;
    vec3 pointHit;
    bool missed;
};

layout(location = 0) rayPayloadNV Payload Result;
const int MAX_RECURSION = 4;

void main() 
{
    const vec2 pixelCenter = vec2(gl_LaunchIDNV.xy) + vec2(0.5);
	const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeNV.xy);
	vec2 d = inUV * 2.0 - 1.0;

	vec4 origin = ubo.viewInverse * vec4(0,0,0,1);
	vec4 target = ubo.projInverse * vec4(d.x, d.y, 1, 1) ;
	vec4 direction = ubo.viewInverse*vec4(normalize(target.xyz), 0);

    vec3 forigin = vec3(origin.xyz);
    vec3 fdir = vec3(direction.xyz);

    const uint rayFlags = gl_RayFlagsOpaqueNV;
    const uint cullMask = 0xFF;
    const float tmin = 0.0f;
    const float tmax = 100.0f;
    const int payloadLocation = 0;

    vec3 finalPointColor = vec3(0);
    float reflectionFactor = 1;

    for (int i = 0; i < MAX_RECURSION; i++)
    {
        traceNV(Scene, rayFlags, cullMask, 0, 0, 0, forigin, tmin, fdir, tmax, payloadLocation);

        vec3 hitColor = Result.pointColor;

        if(!Result.missed)
        {
            if(Result.reflectiveness > 0)
            {
                //reflectionFactor = Result.reflectiveness;
                forigin = Result.pointHit + Result.pointNormal * 0.001f;
                fdir = reflect(fdir, Result.pointNormal);
            }
            finalPointColor += hitColor;
        }
    }
    finalPointColor += vec3(0.2);
    finalPointColor /= MAX_RECURSION;
    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(finalPointColor, 1.0f));
}