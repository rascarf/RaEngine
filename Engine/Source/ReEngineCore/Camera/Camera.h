#pragma once
#include <glm/glm.hpp>

namespace ReEngine
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& Projection):mProjection(Projection){}

        virtual ~Camera() = default;

        [[nodiscard]] const glm::mat4& GetProjection()const{return mProjection;}

    protected:
        glm::mat4 mProjection = glm::mat4(1.0f);
    
    };
}
