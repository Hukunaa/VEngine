#version 460
#extension GL_NV_ray_tracing : require

layout(binding = 0, set = 0) uniform accelerationStructureNV Scene;
layout(location = 0) rayPayloadInNV vec3 ResultColor;
layout(location = 2) rayPayloadNV bool shadowed;
hitAttributeNV vec3 HitAttribs;

struct Vertex
{
    vec3 pos;
    vec3 normal;
};

struct MaterialData
{
    vec4 colorandRoughness;
};

layout(binding = 4, set = 0) buffer Vertices 
{ 
    vec4 v[]; 
} objverts;

layout(binding = 5, set = 0) buffer Indices 
{ 
    uint i[]; 
}indices;

layout(set = 0, binding = 3) buffer Materials
{
    vec4 m[];
}materials;

Vertex getVertex(uint index)
{
    vec4 d0 = objverts.v[2 * index + 0];
	vec4 d1 = objverts.v[2 * index + 1];

	Vertex v;
	v.pos = d0.xyz;
	v.normal = vec3(d0.w, d1.x, d1.y);
    return v;
}
void main() 
{
    const vec3 barycentricCoords = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);
    //const vec3 barycentricCoords = HitAttribs;
    vec3 lightPos = vec3(-4, -5, 2);
    vec3 origin = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
    vec3 direction = normalize(lightPos - origin);

    float tmin = 0.0001;
	float tmax = 100.0;

    ivec3 ind = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1],
                    indices.i[3 * gl_PrimitiveID + 2]);
    
    Vertex v0 = getVertex(ind.x);
    Vertex v1 = getVertex(ind.y);
    Vertex v2 = getVertex(ind.z);

    vec3 normal = normalize(v0.normal * barycentricCoords.x + v1.normal * barycentricCoords.y + v2.normal * barycentricCoords.z);
    normal = vec3(normalize(gl_ObjectToWorldNV * vec4(normal, 0)));
    float dot_product = max(dot(direction, normal), 0.2);
    vec3 color = materials.m[gl_InstanceID].xyz * dot_product;
    /*vec3 color;
    if(gl_InstanceID == 0)
        color = vec3(1, 0, 0);
    if(gl_InstanceID == 1)
        color = vec3(0, 1, 0);*/

    shadowed = true;
    traceNV(Scene, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV|gl_RayFlagsSkipClosestHitShaderNV, 0xFF, 1, 0, 1, origin, tmin, direction, tmax, 2);
    if (shadowed) 
		color *= vec3(0);

    ResultColor = color;
}