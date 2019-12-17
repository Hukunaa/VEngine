#version 460
#extension GL_NV_ray_tracing : require

layout(location = 0) rayPayloadInNV vec3 ResultColor;
hitAttributeNV vec3 HitAttribs;

void main() {
    const vec3 barycentrics = vec3(1.0 - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);

    vec4 distance;
    // gl_InstanceID // THE GOD VARIABLE
    //Will create materials and send the materials array as uniform to the shader, 
    //get the specific object material thanks to its instance id
    /*if(gl_InstanceID == 0)
        distance = vec4(1, 0, 0, 1);
    if(gl_InstanceID == 1)
        distance = vec4(0, 1, 0, 1);*/
    ResultColor = vec3(distance);
}