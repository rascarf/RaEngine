#pragma once
#include <glm/glm.hpp>

namespace ReEngine
{
    class Camera //Runtime Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& Projection):mProjection(Projection){}
        virtual ~Camera() = default;

        [[nodiscard]] const glm::mat4& GetProjection()const{return mProjection;}
    protected:
        glm::mat4 mProjection = glm::mat4(1.0f);
    };

    class SceneCamera : public Camera
    {
    public:
        enum class ProjectionType {Perspective = 0 , Orthographic = 1};
    public:
        SceneCamera();
        SceneCamera(const glm::mat4& Projection):Camera(Projection){}
        virtual ~SceneCamera(){};

        void SetOrthoGraphic(float size,float nearClip,float FarClip );
        void SetPerspective(float VerticalFov,float NearClip,float FarClip);

        float GetPerspectiveVerticalFOV() const { return m_PerspectiveFOV; }
        void SetPerspectiveVerticalFOV(float verticalFov) { m_PerspectiveFOV = verticalFov; ReCalculateProjection(); }
        float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
        void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; ReCalculateProjection(); }
        float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
        void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; ReCalculateProjection(); }

        float GetOrthographicNearClip() const { return m_OrthoGraphicNear; }
        void SetOrthographicNearClip(float nearClip) { m_OrthoGraphicNear = nearClip; ReCalculateProjection(); }
        float GetOrthographicFarClip() const { return m_OrthoGraphicFar; }
        void SetOrthographicFarClip(float farClip) { m_OrthoGraphicFar = farClip; ReCalculateProjection(); }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; ReCalculateProjection(); }
        
        void SetViewportSize(uint32_t width,uint32_t height);

        float GetOrthoGraphicSize()const {return m_OrthoGraphicSize;}
        void SetOrthoGraphicSize(float OrthoGraphicSize){m_OrthoGraphicSize = OrthoGraphicSize; ReCalculateProjection();}

    private:
        void ReCalculateProjection();
    private:
        float m_OrthoGraphicSize = 10.0f;
        float m_OrthoGraphicNear = -1.0f;
        float m_OrthoGraphicFar = 1.0f;
        float mAspectRatio = 1.0f;
        uint32_t mViewPortWidth = 0.0f,mViewPortHeight = 0.0f;

        ProjectionType m_ProjectionType = ProjectionType::Orthographic;

        float m_PerspectiveFOV = glm::radians(45.0f);
        float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;
    };
}
