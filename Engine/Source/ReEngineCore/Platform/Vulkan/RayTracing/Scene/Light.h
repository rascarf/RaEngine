#pragma once
#include "Core/Core.h"
#include "Core/MathDefine.h"
#include "glm/geometric.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#define LIGHT_TYPE_SKY 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_DIRECTIONAL 2
#define LIGHT_TYPE_RECT 3
#define LIGHT_TYPE_SPOT 4
#define LIGHT_TYPE_MAX 5

namespace ReEngine
{
    class Light
    {
    public:
        Light(int InType): type(InType)
        {
            
        }
        
        virtual ~Light()
        {
        }

    public:
        int type;
    };

    class DirectionalLight : public Light
    {
        using Vector3 = glm::vec3;
        
    public:

        DirectionalLight(const Vector3& inDirection, const Vector3& inColor)
            : Light(LIGHT_TYPE_DIRECTIONAL)
            , direction(inDirection)
            , color(inColor)
            , radius(MAX_int32)
        {

        }

        Vector3 direction;
        Vector3 color;
        float radius;
    };

    class RectLight : public Light
    {
    public:

        using Vector3 = glm::vec3;
        
        RectLight(
            const Vector3& inPosition,
            const Vector3& inDirection,
            const Vector3& inTangent,
            const Vector3& inColor,
            float inSourceRadius,
            float inSourceLength,
            float inRadius,
            float inBarnCosAngle,
            float inBarnLength
        )
            : Light(LIGHT_TYPE_RECT)
            , position(inPosition)
            , direction(inDirection)
            , tangent(inTangent)
            , color(inColor)
            , sourceRadius(inSourceRadius)
            , sourceLength(inSourceLength)
            , radius(inRadius)
            , barnCosAngle(inBarnCosAngle)
            , barnLength(inBarnLength)
        {

        }

        Vector3 position;
        Vector3 direction;
        Vector3 tangent;
        Vector3 color;
        float sourceRadius;
        float sourceLength;
        float radius;
        float barnCosAngle;
        float barnLength;
    };

    class SpotLight : public Light
    {

        using Vector3 = glm::vec3;
        
    public:
        SpotLight(const Vector3& inPosition, const Vector3& inDirection, const Vector3& inColor, float inSpotAngles, float inSourceRadius, float inRadius)
            : Light(LIGHT_TYPE_SPOT)
            , position(inPosition)
            , direction(inDirection)
            , color(inColor)
            , spotAngles(inSpotAngles)
            , sourceRadius(inSourceRadius)
            , radius(inRadius)
        {

        }

        Vector3 position;
        Vector3 direction;
        Vector3 color;
        float spotAngles;
        float sourceRadius;
        float radius;
    };

    class PointLight : public Light
    {
        using Vector3 = glm::vec3;
    
    public:

        PointLight(const Vector3& inPosition, const Vector3& inColor, float inSrouceRadius, float inRadius)
            : Light(LIGHT_TYPE_POINT)
            , position(inPosition)
            , color(inColor)
            , sourceRadius(inSrouceRadius)
            , radius(inRadius)
        {

        }

        Vector3 position;
        Vector3 color;
        float sourceRadius;
        float radius;
    };

    class SkyEnvLight : public Light
    {

        using Vector3 = glm::vec3;
        
    public:
        SkyEnvLight(const Vector3& inColor)
            : Light(LIGHT_TYPE_SKY)
            , color(inColor)
        {

        }

        Vector3 color;
    };

    struct LightData
    {
        using Vector4 = glm::vec4;
        using Vector3 = glm::vec3;
        
        Vector4 type = Vector4(0.0,0.0,0.0,1.0);
        Vector4 position = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 normal = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 dPdu = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 dPdv = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 color = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 dimensions = Vector4(0.0,0.0,0.0,1.0);;
        Vector4 attenuation = Vector4(0.0,0.0,0.0,1.0);;

        void SetLight(const DirectionalLight* light)
        {
            type.x = light->type;
            normal = Vector4(-light->direction,1.0);
            color = Vector4(light->color,1.0);
            attenuation.x = light->radius;
        }

        void SetLight(const RectLight* light)
        {
            type.x = light->type;
            position = Vector4(light->position,1.0);
            normal = Vector4(-light->direction,1.0);
            dPdu = Vector4(glm::cross(light->tangent, light->direction),1.0);
            dPdv = Vector4(light->tangent,1.0);
            color = Vector4(light->color,1.0);
            dimensions.x = 2.0f * light->sourceRadius;
            dimensions.y = 2.0f * light->sourceLength;
            dimensions.z = light->barnCosAngle;
            dimensions.w = light->barnLength;
            attenuation.x = light->radius;
        }

        void SetLight(const SpotLight* light)
        {
            type.x = light->type;
            position = Vector4(light->position,1.0);
            normal = Vector4(-light->direction,1.0);
            color = Vector4(4.0f * PI * light->color,1.0f);
            dimensions.x = light->spotAngles;
            dimensions.y = light->sourceRadius;
            attenuation.x = light->radius;
        }

        void SetLight(const PointLight* light)
        {
            type.x = light->type;
            position = Vector4(light->position,1.0f);
            color = Vector4(light->color / (4.0f * PI),1.0f);
            dimensions.x = 0.0f;
            dimensions.y = 0.0f;
            dimensions.z = light->sourceRadius;
            attenuation.x = light->radius;
        }

        void SetLight(const SkyEnvLight* light)
        {
            type.x = light->type;
            color = Vector4(light->color,1.0f);
        }
    };
}


