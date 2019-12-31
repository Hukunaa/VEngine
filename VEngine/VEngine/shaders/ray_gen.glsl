#version 460
#extension GL_NV_ray_tracing : require
#define M_PI 3.1415926535897932384626433832795

layout(set = 0, binding = 0) uniform accelerationStructureNV Scene;
layout(set = 0, binding = 1, rgba8) uniform image2D ResultImage;
layout(set = 0, binding = 2) uniform CamData 
{
    mat4 viewInverse;
    mat4 projInverse;
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


const int MAX_RECURSION = 16;
const int SUPER_SAMPLING = 128;

const int payloadLocation = 0;
const uint rayFlags = gl_RayFlagsOpaqueNV;
const uint cullMask = 0xFF;
const float tmin = 0.0001;
const float tmax = 100;

float seedRand = 0;
float rand() { return fract(sin(seedRand++)*43758.5453123); }

vec3 rgb(vec3 values)
{
    return vec3(values.x / 255.0, values.y / 255.0, values.z / 255.0);
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

/*vec3 random_cos_weighted_hemisphere_direction( const vec3 n, inout float seed) {
  	vec2 r = hash2(seed);
	vec3  uu = normalize(cross(n, abs(n.y) > .5 ? vec3(1.,0.,0.) : vec3(0.,1.,0.)));
	vec3  vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra*cos(6.28318530718*r.x); 
	float ry = ra*sin(6.28318530718*r.x);
	float rz = sqrt(1.-r.y);
	vec3  rr = vec3(rx*uu + ry*vv + rz*n);
    return normalize(rr);
}

vec3 random_in_unit_sphere(inout float seed) 
{
    vec3 h = hash3(seed) * vec3(2.,6.28318530718,1.)-vec3(1,0,0);
    float phi = h.y;
    float r = pow(h.z, 1./3.);
	return r * vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi),cos(phi)),h.x);
}*/

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
        vec3 fdir = normalize(rd + rayHit.matSpecs.y * randomHemisphereDirection(rayHit.pointNormal, seedRand));
        attenuation = rayHit.pointColor;
        origin = forigin;
        dir = fdir;
        return true;
    }
    return false;
}

float getEmitted(Payload ray, vec3 origin)
{
    //TEST RECTANGLE LIGHT
    vec3 position = vec3(-8, -6, 20);
    float sizeX = 12;
    float sizeY = 12;
    float celldivisionFactor = 4;
    float intensity = 1.2;
    float amountOfLight = 0;
    for(float i = 0; i < sizeX; i += sizeX / celldivisionFactor)
    {
        for(float j = 0; j < sizeY; j += sizeY / celldivisionFactor)
        {
            vec3 dir = normalize((vec3(position.x + i, position.y - j, position.z) - ray.pointHit) + 0.5 * randomHemisphereDirection((vec3(position.x + i, position.y + j, position.z) - ray.pointHit), seedRand));
            traceNV(Scene, rayFlags, cullMask, 0, 0, 0, origin, tmin, dir, tmax, payloadLocation);
            Payload shadowResult = Result;

            if(shadowResult.hasHit == false)
                amountOfLight += (intensity); //* dot(dir, ray.pointNormal)); /// (distance(position, ray.pointHit) * (distance(position, ray.pointHit)));

        }
    }
    amountOfLight /= celldivisionFactor * celldivisionFactor;
    return amountOfLight;
}
void main() 
{
    //recursion to iteration
    vec3 color = vec3(0);
    for(int k = 0; k < SUPER_SAMPLING; ++k)
    {
        const vec2 pixelCenter = (vec2(gl_LaunchIDNV.xy) + hash2(seedRand));
        seedRand =  pixelCenter.x * pixelCenter.y * time.t[0];
        const vec2 inUV = pixelCenter/vec2(gl_LaunchSizeNV.xy);
        vec2 d = inUV * 2.0 - 1.0;

        vec4 origin = ubo.viewInverse * vec4(0,0,0,1);
        vec4 target = ubo.projInverse * vec4(d.x, d.y, 1, 1) ;
        vec4 direction = ubo.viewInverse*vec4(normalize(target.xyz), 0);

        vec3 forigin = vec3(origin.xyz);
        vec3 fdir = vec3(direction.xyz);

        vec3 pixelColor = vec3(1);
        float lightAmount = 0;
        Payload hitRecord;
        for(int i = 0; i < MAX_RECURSION; ++i)
        {
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
                        pixelColor *= attenuation * getEmitted(hitRecord, forigin);
                }
                //lightAmount += getEmitted(hitRecord, forigin);
                /*else
                {
                    pixelColor *= (lightAmount / MAX_RECURSION);
                    break;
                }*/
            }
            else
            {
                pixelColor *= vec3(0.75, 0.85, 1);
                break;
            }
        }
        color += pixelColor;
    }
    color /= SUPER_SAMPLING;
    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(color, 1.0));
}