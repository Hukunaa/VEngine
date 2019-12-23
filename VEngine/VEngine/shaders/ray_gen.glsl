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

struct Payload
{
    vec3 pointColor;
    vec3 pointNormal;
    vec3 matSpecs;
    vec3 pointHit;
    //vec3 pointTangent;
    //vec3 biTangent;
};

layout(location = 0) rayPayloadNV Payload Result;
const int MAX_RECURSION = 4;
const int MAX_REFLEXION_PER_RAY = 32;

vec2 hash2(inout float seed) 
{
    return fract(sin(vec2(seed+=0.1,seed+=0.1))*vec2(43758.5453123,22578.1459123));
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
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

/**
 * Generate a uniformly distributed random point on the unit-hemisphere
 * around the given normal vector.
 * 
 * This function can be used to generate reflected rays for diffuse surfaces.
 * Actually, this function can be used to sample reflected rays for ANY surface
 * with an arbitrary BRDF correctly.
 * This is because we always need to solve the integral over the hemisphere of
 * a surface point by using numerical approximation using a sum of many
 * sample directions.
 * It is only with non-lambertian BRDF's that, in theory, we could sample them more
 * efficiently, if we knew in which direction the BRDF reflects the most energy.
 * This would be importance sampling, but care must be taken as to not over-estimate
 * those surfaces, because then our sum for the integral would be greater than the
 * integral itself. This is the inherent problem with importance sampling.
 * 
 * The points are uniform over the sphere and NOT over the projected disk
 * of the sphere, so this function cannot be used when sampling a spherical
 * light, where we need to sample the projected surface of the light (i.e. disk)!
 */
vec3 randomHemispherePoint(vec3 rand, vec3 n) 
{
  vec3 v = randomSpherePoint(rand);
  return v * sign(dot(v, n));
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

    const uint rayFlags = gl_RayFlagsOpaqueNV;
    const uint cullMask = 0xFF;
    const float tmin = 0.0f;
    const float tmax = 100.0f;
    const int payloadLocation = 0;

    vec3 finalPointColor = vec3(0);
    float totalLight = 1;
    int rayDivision = 0;

    float reflectionPower = 1;
    for (int i = 0; i < MAX_RECURSION; i++)
    {
        vec3 primaryColor;
        traceNV(Scene, rayFlags, cullMask, 0, 0, 0, forigin, tmin, fdir, tmax, payloadLocation);
        Payload primaryPayload = Result;

        primaryColor = primaryPayload.pointColor * reflectionPower;


        rayDivision++;
        //IF RAY MISS
        if(primaryPayload.matSpecs.z == 1 || primaryPayload.matSpecs.x <= 0)
        {
            finalPointColor += primaryColor;
            break;
        }

        //REFLECTION PART
        forigin = primaryPayload.pointHit + primaryPayload.pointNormal * 0.001f;
        fdir = reflect(fdir, primaryPayload.pointNormal);
        reflectionPower = 1 - dot(fdir, primaryPayload.pointNormal);

        //RAY FINAL COLOR
        finalPointColor += primaryColor;

    }
    
    finalPointColor /= rayDivision;

    imageStore(ResultImage, ivec2(gl_LaunchIDNV.xy), vec4(finalPointColor, 1.0f));
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