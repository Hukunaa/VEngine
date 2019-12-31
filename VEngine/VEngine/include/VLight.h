#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct VLightData
{
    glm::vec4 position;
    glm::mat4 transform;
    glm::vec3 sizeIntensity;
    glm::vec3 recDirx{1, 0, 0};
    glm::vec3 recDiry{0, 1, 0};
    float dummyFloat{0};

};
class VLight
{
public:
    VLight(): transMat(glm::mat4(1)), rotationMat(glm::mat4(1)), scaleMat(glm::mat4(1)) {}
    ~VLight() = default;

    void SetPosition(glm::vec3 pos)
    {
        const glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        transMat = translate;
        data.transform = glm::transpose(transMat * rotationMat * scaleMat);
    }
    void Translate(glm::vec3 pos)
    {
        const glm::mat4 translate = glm::translate(glm::mat4(1.0f), pos);
        transMat *= translate;
        data.transform = transMat * rotationMat * scaleMat;
    }
    void SetRotation(glm::vec3 angle)
    {
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.x), glm::vec3(1, 0, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.y), glm::vec3(0, 1, 0));
        rotationMat = glm::rotate(rotationMat, glm::radians(angle.z), glm::vec3(0, 0, 1));
        data.transform = glm::transpose(transMat * rotationMat * scaleMat);
    }

    glm::mat4 transMat;
    glm::mat4 rotationMat;
    glm::mat4 scaleMat;
    VLightData data{};
};

