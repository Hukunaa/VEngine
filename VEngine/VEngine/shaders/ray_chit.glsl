#version 460
#extension GL_NV_ray_tracing : require

layout(binding = 0, set = 0) uniform accelerationStructureNV Scene;
layout(binding = 2, set = 0) uniform CamData 
{
    mat4 viewInverse;
    mat4 projInverse;
    vec4 data;
    
} ubo;

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    vec3 matSpecs;
    vec3 pointHit;
    bool hasHit;
};

layout(location = 0) rayPayloadInNV Payload Result;
hitAttributeNV vec3 HitAttribs;

struct Vertex
{
    vec3 pos;
    vec3 normal;
    int id;
};

struct MaterialData
{
    vec4 colorandRoughness;
};

layout(binding = 3, set = 0) buffer Materials
{
    vec4 m[];
}materials;

layout(binding = 4, set = 0) buffer Vertices 
{ 
    vec4 v[]; 
} objverts;

layout(binding = 6, set = 0) buffer NbTriangles
{
    int nb[];
}trianglesNb;

Vertex getVertex(uint index)
{
    vec4 d0 = objverts.v[2 * index];
	vec4 d1 = objverts.v[2 * index + 1];
	Vertex v;
	v.pos = d0.xyz;
	v.normal = d1.xyz;
    v.id = int(d0.w);
    return v;
}
void main() 
{
    //BARYCENTRICS COORDS
    const vec3 barycentricCoords = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);

    //TRIANGLE VERTICES V0, V1, V2
    int triangle = 0;
    for(int i = 0; i < gl_InstanceID; ++i)
    {
        triangle += trianglesNb.nb[i];
    }
    Vertex v0 = getVertex((triangle + gl_PrimitiveID) * 3);
    Vertex v1 = getVertex((triangle + gl_PrimitiveID) * 3 + 1);
    Vertex v2 = getVertex((triangle + gl_PrimitiveID) * 3 + 2);

    //CALCULATE SURFACE NORMAL
    vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    normal = vec3(normalize(gl_ObjectToWorldNV * vec4(normal, 0)));

    //GET MATERIAL DATA FROM OBJECT
    Result.matSpecs.z = 0;
    vec4 matData = materials.m[2 * gl_InstanceID];
    vec4 matData2 = materials.m[2 * gl_InstanceID + 1];
    vec3 origin = gl_WorldRayOriginNV + (gl_WorldRayDirectionNV * gl_HitTNV) + normal * 0.0001;

    Result.hasHit = true;
    Result.pointHit = origin;
    Result.pointNormal = normal;
    Result.matSpecs.x = matData.w;
    Result.matSpecs.y = matData2.x;
    Result.matSpecs.z = matData2.y;
    Result.pointColor = matData.xyz;
    //Result.lightReceived = color;
}