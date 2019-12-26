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


const int MAX_RECURSION = 32;
const int MAX_INDIRECT = 4;
const int payloadLocation = 0;
const uint rayFlags = gl_RayFlagsOpaqueNV;
const uint cullMask = 0xFF;
const float tmin = 0.0001;
const float tmax = 100;

float seedRand = 0;
float rand() { return fract(sin(seedRand++)*43758.5453123); }

uint base_hash(uvec2 p) 
{
    p = 1103515245U*((p >> 1U)^(p.yx));
    uint h32 = 1103515245U*((p.x)^(p.y>>3U));
    return h32^(h32 >> 16);
}
vec3 hash3(inout float seed) 
{
    uint n = base_hash(floatBitsToUint(vec2(seed+=.1,seed+=.1)));
    uvec3 rz = uvec3(n, n*16807U, n*48271U);
    return vec3(rz & uvec3(0x7fffffffU))/float(0x7fffffff);
}

vec3 randomSpherePoint(vec3 rand) {
  float ang1 = (rand.x + 1.0) * M_PI; // [-1..1) -> [0..2*PI)
  float u = rand.y; // [-1..1), cos and acos(2v-1) cancel each other out, so we arrive at [-1..1)
  float u2 = u * u;
  float sqrt1MinusU2 = sqrt(1.0 - u2);
  float x = sqrt1MinusU2 * cos(ang1);
  float y = sqrt1MinusU2 * sin(ang1);
  float z = u;
  return vec3(x, y, z);
}

 vec3 randomCosineWeightedHemispherePoint(vec3 rand, vec3 n) 
 {
  float r = rand.x * 0.5 + 0.5; // [-1..1) -> [0..1)
  float angle = (rand.y + 1.0) * M_PI; // [-1..1] -> [0..2*PI)
  float sr = sqrt(r);
  vec2 p = vec2(sr * cos(angle), sr * sin(angle));
  /*
   * Unproject disk point up onto hemisphere:
   * 1.0 == sqrt(x*x + y*y + z*z) -> z = sqrt(1.0 - x*x - y*y)
   */
  vec3 ph = vec3(p.xy, sqrt(1.0 - p*p));
  /*
   * Compute some arbitrary tangent space for orienting
   * our hemisphere 'ph' around the normal. We use the camera's up vector
   * to have some fix reference vector over the whole screen.
   */
  vec3 tangent = normalize(rand);
  vec3 bitangent = cross(tangent, n);
  tangent = cross(bitangent, n);
  
  /* Make our hemisphere orient around the normal. */
  return tangent * ph.x + bitangent * ph.y + n * ph.z;
}

vec3 random_in_unit_sphere(inout float seed) 
{
    vec3 h = hash3(seed) * vec3(2.,6.28318530718,1.)-vec3(1,0,0);
    float phi = h.y;
    float r = pow(h.z, 1./3.);
	return r * vec3(sqrt(1.-h.x*h.x)*vec2(sin(phi),cos(phi)),h.x);
}

vec3 randomHemispherePoint(vec3 rand, vec3 n) 
{
  vec3 v = randomSpherePoint(rand);
  return v * sign(dot(v, n));
}

bool getScattering(Payload rayHit, inout vec3 origin, inout vec3 dir, inout vec3 attenuation)
{
    if(rayHit.matSpecs.x == 1)
    {
        vec3 forigin = rayHit.pointHit;
        vec3 fdir = normalize(rayHit.pointNormal + random_in_unit_sphere(seedRand));
        attenuation = rayHit.pointColor;
        origin = forigin;
        dir = fdir;
        return true;
    }
    if(rayHit.matSpecs.x == 2)
    {
        vec3 rd = reflect(dir, rayHit.pointNormal);
        vec3 forigin = rayHit.pointHit;
        vec3 fdir = normalize(rd + rayHit.matSpecs.y * random_in_unit_sphere(seedRand));
        attenuation = rayHit.pointColor;
        origin = forigin;
        dir = fdir;
        return true;
    }
    return false;
}

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

    //recursion to iteration
    vec3 finalPointColor = vec3(1);
    seedRand =  gl_LaunchIDNV.x * gl_LaunchIDNV.y * time.t[0];
    Payload hitRecord;
    for(int i = 0; i < MAX_RECURSION; ++i)
    {
        traceNV(Scene, rayFlags, cullMask, 0, 0, 0, forigin, tmin, fdir, tmax, payloadLocation);
        hitRecord = Result;
        if(hitRecord.hasHit)
        {
            vec3 attenuation;
            if(getScattering(hitRecord, forigin, fdir, attenuation))
                finalPointColor *= attenuation;
            else
                finalPointColor *= 0;
        }
        else
        {
            float t = .5 * fdir.y + .5;
            finalPointColor *= mix(vec3(1),vec3(0.5,0.7,1), t);
            //finalPointColor *= 0.3;
            break;
        }
    }
    //finalPointColor /= MAX_RECURSION;

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(finalPointColor, 1.0));
}


/*void fresnel(const vec3 I, const vec3 N, const float ior, inout float kr) 
{ 
    float cosi = clamp(-1, 1, dot(I, N)); 
    float etai = 1, etat = ior; 
    if (cosi > 0) 
    { 
        float tmp = etai;
        etai = etat;
        etat = tmp;
    } 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrt(max(0, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) 
    { 
        kr = 1; 
    } 
    else 
    { 
        float cost = sqrt(max(0.f, 1 - sint * sint)); 
        cosi = abs(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}*/


        /*forigin = Result.pointHit + Result.pointNormal * 0.001f;
        fdir = reflect(fdir, Result.pointNormal);*/


        //if(primaryPayload.matSpecs.x > 0)
        //{
           /* vec3 reflexionColor;
            float shadowpercentage = MAX_REFLEXION_PER_RAY;
            for(int i = 0; i < MAX_REFLEXION_PER_RAY; ++i)
            {
                vec3 start = primaryPayload.pointHit + primaryPayload.pointNormal * 0.001f;

                float rdx = rand(vec2(primaryPayload.pointHit.x, primaryPayload.pointHit.y));
                float rdy = rand(vec2(primaryPayload.pointHit.y, primaryPayload.pointHit.z));
                float rdz = rand(vec2(primaryPayload.pointHit.x + gl_LaunchIDNV.x, primaryPayload.pointHit.y + gl_LaunchIDNV.y));

                vec3 dir = normalize(randomHemispherePoint(vec3(rdx, rdy, rdz), primaryPayload.pointNormal));
                traceNV(Scene, rayFlags, cullMask, 0, 0, 0, start, tmin, dir, tmax, payloadLocation);
                Payload reflexionPayload = Result;

                if(Result.matSpecs.z == 0)
                    shadowpercentage -= dot(dir, primaryPayload.pointNormal);
                //reflexionColor += reflexionPayload.pointColor * dot(primaryPayload.pointNormal, dir);
                //totalLight *= Result.matSpecs.x;

            }
            primaryColor *= (shadowpercentage / MAX_REFLEXION_PER_RAY);
            finalPointColor += primaryColor;*/
            //reflexionColor /= MAX_REFLEXION_PER_RAY;
            //rayDivision++;
        //}
        //else
        //    break;


        /*if(primaryPayload.matSpecs.x > 0)
        {
            //REFLECTION PART
            forigin = primaryPayload.pointHit + primaryPayload.pointNormal * 0.01;
            fdir = reflect(fdir, primaryPayload.pointNormal);
            primaryColor *= 1 - dot(fdir, primaryPayload.pointNormal) * primaryPayload.matSpecs.x;
        }*/

        //INDIRECT LIGHT
        /*vec3 indirectColor = vec3(0);
        float division = 1;
        for(int k = 0; k < MAX_INDIRECT; ++k)
        {
            float rx = rand(vec2(primaryPayload.pointHit.x + k, primaryPayload.pointHit.y + k));
            float ry = rand(vec2(primaryPayload.pointHit.x + k * 3, primaryPayload.pointHit.y + k * 3));
            float rz = rand(vec2(primaryPayload.pointHit.x + k * 2, primaryPayload.pointHit.y + k * 2));
            vec3 rayOrigin = forigin;
            vec3 rayDir = randomCosineWeightedHemispherePoint(vec3(rx, 1250, rz), primaryPayload.pointNormal);
            traceNV(Scene, rayFlags, cullMask, 0, 0, 0, rayOrigin, tmin, rayDir, tmax, 0);
            indirectColor += dot(rayDir, primaryPayload.pointNormal) * Result.pointColor;
            division += 1;
        }
        indirectColor /= division;*/

        //RAY FINAL COLOR
        //finalPointColor += (primaryColor + indirectColor);