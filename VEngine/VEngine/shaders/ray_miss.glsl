#version 460
#extension GL_NV_ray_tracing : require

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    float reflectiveness;
    vec3 pointHit;
    bool missed;
};

layout(location = 0) rayPayloadInNV Payload Result;
void main() 
{
    Result.missed = true;
    Result.pointColor = vec3(0.0, 0.5, 0.5);
}