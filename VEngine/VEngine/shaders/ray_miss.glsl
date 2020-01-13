#version 460
#extension GL_NV_ray_tracing : require

struct ObjInfo
{
    vec3 albedo;
    vec3 normal;
    vec3 hitPoint;
    vec3 material;
    bool isValid;
};

struct Payload
{
    ObjInfo objInfos;
};

layout(location = 0) rayPayloadInNV Payload payloadData;

void main() 
{
    payloadData.objInfos.isValid = false;
    payloadData.objInfos.albedo = vec3(0.53, 0.81, 0.92);
    payloadData.objInfos.hitPoint = vec3(0);
    payloadData.objInfos.normal = vec3(0);
    payloadData.objInfos.material = vec3(0);
}