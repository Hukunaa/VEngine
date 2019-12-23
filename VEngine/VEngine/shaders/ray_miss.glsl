#version 460
#extension GL_NV_ray_tracing : require

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    vec3 matSpecs;
    vec3 pointHit;
    //vec3 pointTangent;
};

layout(location = 0) rayPayloadInNV Payload Result;
void main() 
{
    Result.matSpecs.z = 1;
    Result.pointColor = vec3(0.529, 0.8, 0.92);
}