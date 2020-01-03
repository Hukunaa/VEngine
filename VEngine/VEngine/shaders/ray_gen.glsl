#version 460
#extension GL_NV_ray_tracing : require

#define M_PI 3.1415926535897932384626433832795
#define GAMMA 2.2
#define INV_GAMMA 0.45454545454545453
#define BOUNCE_COUNT 6

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

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    vec3 matSpecs;
    vec3 pointHit;
    bool hasHit;
    //float attenuation;
};

layout(location = 0) rayPayloadNV Payload Result;
layout(location = 2) rayPayloadNV bool shadowed;

const int payloadLocation = 0;
const uint rayFlags = gl_RayFlagsOpaqueNV;
const uint cullMask = 0xFF;
const float tmin = 0.0001;
const float tmax = 10000;

float seedRand = 0;
float rand() { return fract(sin(seedRand++)*43758.5453123); }

vec3 SRGBToLinear(vec3 color) {
  return pow(color, vec3(INV_GAMMA));
}

vec3 Uncharted2ToneMapping(vec3 color) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  float exposure = 2.0;
  color *= exposure;
  color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
  float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
  return SRGBToLinear(color / white);
}

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

vec3 cosWeightedRandomHemisphereDirection( const vec3 n, inout float seed ) {
  	vec2 r = hash2(seed);
    
	vec3  uu = normalize( cross( n, vec3(0.0,1.0,1.0) ) );
	vec3  vv = cross( uu, n );
	
	float ra = sqrt(r.y);
	float rx = ra*cos(6.2831*r.x); 
	float ry = ra*sin(6.2831*r.x);
	float rz = sqrt( 1.0-r.y );
	vec3  rr = vec3( rx*uu + ry*vv + rz*n );
    
    return normalize( rr );
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

vec3 getEmitted(Payload rayHit)
{
    if(rayHit.matSpecs.x == 3)
        return rayHit.pointColor * rayHit.matSpecs.z;
    else
        return vec3(0);
}

bool getScattering(Payload rayHit, inout vec3 origin, inout vec3 dir, inout vec3 attenuation)
{
    //LAMBERTIAN
    if(rayHit.matSpecs.x == 1)
    {
        vec3 forigin = rayHit.pointHit;
        vec3 fdir = normalize(randomHemisphereDirection(rayHit.pointNormal, seedRand));
        attenuation = rayHit.pointColor;
        origin = forigin;
        dir = fdir;
        return true;
    }
    //METAL
    if(rayHit.matSpecs.x == 2)
    {
        vec3 rd = reflect(dir, rayHit.pointNormal);
        vec3 forigin = rayHit.pointHit;
        vec3 fdir = normalize(rd + (1 - rayHit.matSpecs.y) * randomHemisphereDirection(rayHit.pointNormal, seedRand));
        attenuation = rayHit.pointColor;
        origin = forigin;
        dir = fdir;
        return true;
    }
    return false;
}

const int SHADOW_RES = 6;
float getEmitted(Payload ray, vec3 origin)
{
    //TEST SPHERICAL LIGHT
    vec3 position = vec3(0, -400, -1200);
    float radius = 2;
    float amountOfLight = 0;
    float intensity = 10;
    for(int i = 0; i < SHADOW_RES; ++i)
    {
        vec3 rd = normalize(randomSphereDirection(seedRand));
        vec3 pointOnSphere = position + rd * radius;
        vec3 dir = normalize(pointOnSphere - ray.pointHit);
        shadowed = true;
        traceNV(Scene, gl_RayFlagsTerminateOnFirstHitNV | gl_RayFlagsOpaqueNV| gl_RayFlagsSkipClosestHitShaderNV, cullMask, 0, 0, 0, ray.pointHit, tmin, dir, tmax, 2);
        if(!shadowed)
            amountOfLight += (intensity * dot(dir, ray.pointNormal)); /// (distance(position, ray.pointHit) * distance(position, ray.pointHit));
    }

    amountOfLight /= SHADOW_RES;
    return amountOfLight;

    //TEST AREA LIGHT
    /*float sizeX = 6;
    float sizeY = 6;
    float celldivisionFactor = 2;
    float intensity = 1;
    float amountOfLight = 0;
    for(float i = 0; i < sizeX; i += sizeX / celldivisionFactor)
    {
        for(float j = 0; j < sizeY; j += sizeY / celldivisionFactor)
        {
            vec3 cellDir = vec3(position.x + i, position.y - j, position.z) - ray.pointHit;
            vec3 dir = normalize(cellDir + randomHemisphereDirection(cellDir, seedRand));
            traceNV(Scene, rayFlags, cullMask, 0, 0, 0, origin, tmin, dir, tmax, payloadLocation);
            Payload shadowResult = Result;

            if(shadowResult.hasHit == false)
                amountOfLight += (intensity); //* dot(dir, ray.pointNormal)); /// (distance(position, ray.pointHit) * (distance(position, ray.pointHit)));

        }
    }*/
}
void main() 
{
    //recursion to iteration
    vec3 color = vec3(0);
    for(int k = 0; k < ubo.data.x; ++k)
    {
        const vec2 pixelCenter = (vec2(gl_LaunchIDNV.xy) + hash2(seedRand));
        seedRand =  pixelCenter.x * pixelCenter.y * time.t[0];
        const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeNV.xy);
        vec2 d = inUV * 2.0 - 1.0;

        vec4 origin = inverse(ubo.viewInverse) * vec4(0,0,0,1);
        vec4 target = inverse(ubo.projInverse) * vec4(d.x, d.y, -1, 1) ;
        vec4 direction = inverse(ubo.viewInverse) * vec4(normalize(target.xyz), 0);

        vec3 forigin = vec3(origin.xyz);
        vec3 fdir = vec3(direction.xyz);

        vec3 pixelColor = vec3(0);
        float lightAmount = 0;
        Payload hitRecord;
        int recursionTimes = 0;
        for(int i = 0; i < BOUNCE_COUNT; ++i)
        {
            recursionTimes++;
            traceNV(Scene, rayFlags, cullMask, 0, 0, 0, forigin, tmin, fdir, tmax, payloadLocation);
            hitRecord = Result;
            if(hitRecord.hasHit)
            {
                vec3 attenuation;
                if(getScattering(hitRecord, forigin, fdir, attenuation))
                {
                    if(i == 0)
                        pixelColor = attenuation * getEmitted(hitRecord, forigin);
                    else
                        pixelColor += attenuation * getEmitted(hitRecord, forigin);
                }
                //lightAmount += getEmitted(hitRecord, forigin);
               /* else
                {
                     pixelColor += vec3(0.75, 0.85, 1);//* getEmitted(hitRecord, forigin);
                    break;
                }*/
            }
            else
            {
                if(i == 0)
                    pixelColor = hitRecord.pointColor;
                else
                {
                    pixelColor +=  hitRecord.pointColor * 0.2;
                    recursionTimes++;
                }
                break;
            }
        }
        color += pixelColor; /// recursionTimes;
        //color *= lightAmount;
    }
    //color /= ubo.sampleLevel;
    float accumulate;
    if(ubo.data.x != ubo.data.y)
        accumulate = 0;
    else
        accumulate = 1;
    vec3 accumulation = imageLoad(accImage, ivec2(gl_LaunchIDNV.xy)).rgb;
    vec3 finalColor = mix(accumulation, color, 1 / (ubo.data.y + 1.0));
    //vec3 accumulation = mix(imageLoad(accImage, ivec2(gl_LaunchIDNV.xy)).rgb, color, 1 / (ubo.data.y + 1.0));
    //vec3 accumulation = (imageLoad(accImage, ivec2(gl_LaunchIDNV.xy)).rgb + color) / ubo.data.y;
    //vec3 accumulation = imageLoad(accImage, ivec2(gl_LaunchIDNV.xy)).rgb * float(accumulate) + color;
    //color = accumulation * (1 / ubo.data.y);
    //finalColor = Uncharted2ToneMapping(finalColor);
    
    imageStore(accImage, ivec2(gl_LaunchIDNV.xy), vec4(finalColor, 0));

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(finalColor, 0.0));
}