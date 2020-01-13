#version 460
#extension GL_NV_ray_tracing : require

#define M_PI 3.1415926535897932384626433832795
#define GAMMA 2.2
#define INV_GAMMA 0.45454545454545453

#define ENABLE_REFLECTIONS
//#define ENABLE_HARD_SHADOWS
#define ENABLE_SOFT_SHADOWS
#define ENABLE_GI

#define MAX_REFLECTIONS_RECURSION 2
#define SHADOW_SAMPLES 4

//#define ENABLE_ACCUMULATION

layout(set = 0, binding = 0) uniform accelerationStructureNV Scene;
layout(set = 0, binding = 1, rgba8) uniform image2D ResultImage;
layout(set = 0, binding = 7, rgba32f) uniform image2D accImage;
layout(set = 0, binding = 2) uniform CamData 
{
    mat4 viewInverse;
    mat4 projInverse;
    vec4 data;
} ubo;

layout(set = 0, binding = 5) uniform Time
{
    float t[];
}time;


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

layout(location = 0) rayPayloadNV Payload payloadData;
layout(location = 2) rayPayloadNV bool shadowed;

const uint rayFlags = gl_RayFlagsOpaqueNV;
const uint cullMask = 0xFF;
const float tmin = 0.0001;
const float tmax = 150;
float seedRand = 0;

//float rand() { return fract(sin(seedRand++)*43758.5453123); }

uint base_hash(uvec2 p) 
{
    p = 1103515245U*((p >> 1U)^(p.yx));
    uint h32 = 1103515245U*((p.x)^(p.y>>3U));
    return h32^(h32 >> 16);
}

vec2 hash2(inout float seed) 
{
    uint n = base_hash(floatBitsToUint(vec2(seed+=.1,seed+=.1)));
    uvec2 rz = uvec2(n, n*48271U);
    return vec2(rz.xy & uvec2(0x7fffffffU))/float(0x7fffffff);
}

vec3 hash3(inout float seed) 
{
    uint n = base_hash(floatBitsToUint(vec2(seed+=.1,seed+=.1)));
    uvec3 rz = uvec3(n, n*16807U, n*48271U);
    return vec3(rz & uvec3(0x7fffffffU))/float(0x7fffffff);
}

vec4 cosWeightedRandomHemisphereDirection( const vec3 n, inout float seed, const vec3 rayDir) {
  	vec2 r = hash2(seed);
    
	vec3  uu = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  vv = cross( uu, n );
	
	float ra = sqrt(r.y);
	float rx = ra*cos(6.2831*r.x); 
	float ry = ra*sin(6.2831*r.x);
	float rz = sqrt( 1.0-r.y );
	vec3  rr = vec3( rx*uu + ry*vv + rz*n );
    
    return normalize( vec4(rr, dot(n, -rayDir)) );
}

vec3 randomSphereDirection(inout float seed) {
    vec2 h = hash2(seed) * vec2(2.,6.28318530718)-vec2(1,0);
    float phi = h.y;
	return vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi),cos(phi)),h.x);
}

vec3 randomHemisphereDirection( const vec3 n, inout float seed ) {
	vec3 dr = randomSphereDirection(seed);
	return dot(dr,n) * dr;
}

bool getScattering(Payload rayHit, inout vec3 origin, inout vec3 dir, inout vec3 attenuation, inout float cosTheta)
{
    //LAMBERTIAN
    /*if(rayHit.matSpecs.x == 1)
    {
        vec3 forigin = rayHit.pointHit;
        vec4 fdir = normalize(cosWeightedRandomHemisphereDirection(rayHit.pointNormal, seedRand, dir));
        attenuation = rayHit.pointColor;
        cosTheta = fdir.w;
        origin = forigin;
        dir = fdir.xyz;
        return true;
    }
    //METAL
    if(rayHit.matSpecs.x == 2)
    {
        vec3 rd = reflect(dir, rayHit.pointNormal);
        vec3 forigin = rayHit.pointHit;
        vec3 fdir = normalize(rd + (1 - rayHit.matSpecs.y) * randomHemisphereDirection(rayHit.pointNormal, seedRand));
        attenuation = rayHit.pointColor;
        cosTheta = 1;
        origin = forigin;
        dir = fdir;
        return true;
    }*/
    return false;
}

/*float getEmitted(Payload ray, vec3 origin)
{
     //TEST SPHERICAL LIGHT
    float amountOfLight = 0;
    float intensity = 20;
    for(int i = 0; i < SHADOW_RES; ++i)
    {
        vec3 rd = normalize(randomSphereDirection(seedRand));
        vec3 pointOnSphere = position + rd * radius;
        vec3 dir = normalize(pointOnSphere - ray.pointHit);
        shadowed = true;
        traceNV(Scene, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV| gl_RayFlagsSkipClosestHitShaderNV, cullMask, 0, 0, 0, ray.pointHit, tmin, dir, tmax, 2);
        if(!shadowed)
            amountOfLight += (intensity); /// (distance(position, ray.pointHit) * distance(position, ray.pointHit));
    }

    amountOfLight /= SHADOW_RES;
    return amountOfLight;

}
*/

ObjInfo GetObjectInfo(vec3 origin, vec3 dir)
{
    traceNV(Scene, rayFlags, cullMask, 0, 0, 0, origin, tmin, dir, tmax, 0);
    ObjInfo info = payloadData.objInfos;
    return info;
}

float GetLightIntensity(ObjInfo obj)
{
    float totalLight = 0;
    float intensity = 1;
    vec3 lightPos = vec3(0, -200, -1200);
    float radius = 100;

    #ifdef ENABLE_HARD_SHADOWS
        vec3 dir = normalize(lightPos - obj.hitPoint);
        shadowed = true;
        traceNV(Scene, rayFlags | gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsSkipClosestHitShaderNV, cullMask, 0, 0, 0, obj.hitPoint, tmin, dir, 100, 2);
        if(!shadowed)
            totalLight += intensity * dot(obj.normal, dir);
    #endif

    #ifdef ENABLE_SOFT_SHADOWS
        for(int i = 0; i  < SHADOW_SAMPLES; ++i)
        {
            vec3 rd = normalize(randomSphereDirection(seedRand));
            vec3 pointOnSphere = lightPos + rd * radius;
            vec3 dir = normalize(pointOnSphere - obj.hitPoint);
            shadowed = true;
            traceNV(Scene, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV| gl_RayFlagsSkipClosestHitShaderNV, cullMask, 0, 0, 0, obj.hitPoint, tmin, dir, tmax, 2);
            if(!shadowed)
                totalLight += (intensity); /// (distance(position, ray.pointHit) * distance(position, ray.pointHit));
        }
        totalLight /= SHADOW_SAMPLES;
    #endif

    
    return totalLight;
}

ObjInfo GetReflection(ObjInfo obj, inout vec3 origin, inout vec3 dir, inout bool ContinueReflection)
{
    vec3 reflection = reflect(dir, obj.normal);
    vec3 direction = normalize(reflection + (1 - obj.material.y) * randomHemisphereDirection(obj.normal, seedRand));
    traceNV(Scene, rayFlags | gl_RayFlagsTerminateOnFirstHitNV, cullMask, 0, 0, 0, origin, tmin, direction, tmax, 0);
    ObjInfo reflectedObj = payloadData.objInfos;

    //Check if object reflected is reflective itself, if not, stop the reflection recursion
    ContinueReflection = true;

    if(reflectedObj.material.x != 2 || !reflectedObj.isValid)
        ContinueReflection = false;
    else
    {
        origin = reflectedObj.hitPoint;
        dir = direction;
    }

    return reflectedObj;
}

void main() 
{
    vec3 color = vec3(0);
    float lightIncoming = 0;

    float samples = 1;
    for(int k = 0; k < samples; ++k)
    {
        //+ hash2(seedRand))
        const vec2 pixelCenter = vec2(gl_LaunchIDNV.xy);
        seedRand =  pixelCenter.x * pixelCenter.y * (k + 1);

        const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeNV.xy);
        vec2 d = inUV * 2.0 - 1.0;

        vec4 origin = inverse(ubo.viewInverse) * vec4(0,0,0,1);
        vec4 target = inverse(ubo.projInverse) * vec4(d.x, d.y, -1, 1) ;
        vec4 direction = inverse(ubo.viewInverse) * vec4(normalize(target.xyz), 0);

        vec3 forigin = vec3(origin.xyz);
        vec3 fdir = vec3(direction.xyz);

        float lambertPDF = 1 / (2 * M_PI);

        ObjInfo object;
        object = GetObjectInfo(forigin, fdir);
        if(object.isValid)
        {
            //ALL FURTHER CALCULATIONS ARE HERE
            lightIncoming = GetLightIntensity(object);
            float cosT = dot(-fdir, object.normal);
            vec3 FbaseColor = (object.albedo  * (1 / M_PI) ) * lightIncoming;
            vec3 FreflectionColor = vec3(0);
            #ifdef ENABLE_REFLECTIONS
                if(object.material.x == 2)
                {
                    vec3 reflectionColor = vec3(1);
                    vec3 ori = object.hitPoint;
                    vec3 dir = fdir;
                    ObjInfo reflectionObject = object;
                    for(int i = 0; i < MAX_REFLECTIONS_RECURSION; ++i)
                    {
                        bool continueReflections;
                        ObjInfo refObj = GetReflection(reflectionObject, ori, dir, continueReflections);
                        reflectionObject = refObj;
                        if(reflectionObject.isValid)
                            reflectionColor *= reflectionObject.albedo * GetLightIntensity(reflectionObject);
                        else
                            reflectionColor *= vec3(0.53, 0.81, 0.92);

                        if(!continueReflections)
                            break;

                    }
                    FreflectionColor = reflectionColor;
                }
            #endif

            color = FbaseColor * (cosT / lambertPDF) * (1 - object.material.y) + FreflectionColor * object.material.y;

        }
        else
        {
            color = vec3(0.53, 0.81, 0.92);
        }
        
    }

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(color, 0.0));
}