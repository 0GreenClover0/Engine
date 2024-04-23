#pragma once
#include "AnimXForm.h"
#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"
#include "glm/glm.hpp"
#include "imgui_impl/imgui_impl_opengl3_loader.h"

inline glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = (GLfloat)from->a1; to[0][1] = (GLfloat)from->b1;  to[0][2] = (GLfloat)from->c1; to[0][3] = (GLfloat)from->d1;
    to[1][0] = (GLfloat)from->a2; to[1][1] = (GLfloat)from->b2;  to[1][2] = (GLfloat)from->c2; to[1][3] = (GLfloat)from->d2;
    to[2][0] = (GLfloat)from->a3; to[2][1] = (GLfloat)from->b3;  to[2][2] = (GLfloat)from->c3; to[2][3] = (GLfloat)from->d3;
    to[3][0] = (GLfloat)from->a4; to[3][1] = (GLfloat)from->b4;  to[3][2] = (GLfloat)from->c4; to[3][3] = (GLfloat)from->d4;

    return to;
}

inline glm::vec3 aiPosToGLMVec3(const aiVector3D& aiVec3)
{
    glm::vec3 glmVec3;

    glmVec3.x = aiVec3.x;
    glmVec3.y = aiVec3.y;
    glmVec3.z = aiVec3.z;

    return glmVec3;
}

inline glm::quat aiQuatToGLMQuat(const aiQuaternion& aiQuat)
{
    glm::quat glmQuat;

    glmQuat.w = aiQuat.w;
    glmQuat.x = aiQuat.x;
    glmQuat.y = aiQuat.y;
    glmQuat.z = aiQuat.z;

    return glmQuat;
}

inline xform multiplyXForms(const xform& xform1, const xform& xform2)
{
    xform result;
    result.position = xform1.position + xform2.position;
    result.rotation = xform1.rotation * xform2.rotation;
    return result;
}

inline xform operator*(const xform& xform1, const xform& xform2)
{
    return multiplyXForms(xform1, xform2);
}
