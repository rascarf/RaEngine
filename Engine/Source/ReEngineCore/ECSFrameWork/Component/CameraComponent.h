#pragma once
#include "Component.h"
#include "Camera/Camera.h"
#include "Camera/OrthographicCamera.h"

namespace ReEngine
{
    class CameraComponnet : ComponentBase
    {
    public:
        CameraComponnet() = default;
        CameraComponnet(const CameraComponnet&) = default;
        CameraComponnet(const glm::mat4& ProjectionMatrix):Camera(ProjectionMatrix){}

    public:
        Camera Camera;
        bool Primary = true; // TODO: think about moving to Scene
    };
    


    
}
