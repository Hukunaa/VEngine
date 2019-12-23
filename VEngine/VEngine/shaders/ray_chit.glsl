#version 460
#extension GL_NV_ray_tracing : require

layout(binding = 0, set = 0) uniform accelerationStructureNV Scene;
layout(binding = 2, set = 0) uniform CamData 
{
    mat4 viewInverse;
    mat4 projInverse;
} ubo;

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    vec3 matSpecs;
    vec3 pointHit;
    //vec3 pointTangent;
    //vec3 biTangent;
};

layout(location = 0) rayPayloadInNV Payload Result;

layout(location = 2) rayPayloadNV bool shadowed;
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

layout(binding = 5, set = 0) buffer Indices 
{ 
    uint i[]; 
}indices;

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
    Result.matSpecs.z = 0;
    vec4 matData = materials.m[2 * gl_InstanceID];
    vec4 matData2 = materials.m[2 * gl_InstanceID + 1];

    const vec3 barycentricCoords = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    vec3 lightPos = vec3(-4, -6, 1);
    vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
    vec3 direction = normalize(lightPos - origin);

    float tmin = 0.0001;
	float tmax = 100.0;

    int triangle = 0;
    for(int i = 0; i < gl_InstanceID; ++i)
    {
        triangle += trianglesNb.nb[i];
    }
    Vertex v0 = getVertex((triangle + gl_PrimitiveID) * 3);
    Vertex v1 = getVertex((triangle + gl_PrimitiveID) * 3 + 1);
    Vertex v2 = getVertex((triangle + gl_PrimitiveID) * 3 + 2);

    vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    normal = vec3(normalize(gl_ObjectToWorldNV * vec4(normal, 0)));
    float dot_product = max(dot(direction, normal), 0.2);
    vec3 color = matData.xyz * dot_product;
    
    shadowed = true;
    traceNV(Scene, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV| gl_RayFlagsSkipClosestHitShaderNV, 0xFF, 1, 0, 1, origin, tmin, direction, tmax, 2);
    if (shadowed) 
		color *= vec3(0.2);

    //Result.pointTangent = cross(normalize(v0 - v1), normal);
    //Result.biTangent = cross(Result.pointTangent, normal);
    Result.pointHit = origin;
    Result.pointNormal = normal;
    Result.matSpecs.x = matData.w;
    Result.matSpecs.y = matData2.x;
    Result.matSpecs.z = matData2.y;

    /*if(gl_InstanceID == v0.id)
        Result.pointColor = vec3(0, 1, 0);
    else
        Result.pointColor = vec3(1, 0, 0);*/

    Result.pointColor = color;
}