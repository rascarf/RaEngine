#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"

namespace ReEngine
{
    SceneCamera::SceneCamera()
    {
    }

    void SceneCamera::SetOrthoGraphic(float size, float nearClip, float FarClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoGraphicFar = FarClip;
        m_OrthoGraphicNear = nearClip;
        m_OrthoGraphicSize = size;
        ReCalculateProjection();
    }

    void SceneCamera::SetPerspective(float VerticalFov, float NearClip, float FarClip)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_PerspectiveFOV = VerticalFov;
        m_PerspectiveNear = NearClip;
        m_PerspectiveFar = FarClip;
        ReCalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
        mViewPortWidth = width;
        mViewPortHeight = height;
        mAspectRatio = (float) width / (float) height;
        ReCalculateProjection();
    }

    void SceneCamera::ReCalculateProjection()
        {if (m_ProjectionType == ProjectionType::Perspective)
        {
            mProjection = glm::perspective(m_PerspectiveFOV, mAspectRatio, m_PerspectiveNear, m_PerspectiveFar);
        }
        else
        {
            float OrthoLeft = -m_OrthoGraphicSize * mAspectRatio * 0.5;
            float OrthoRight = m_OrthoGraphicSize * mAspectRatio * 0.5;
            float OrthoBottom = -m_OrthoGraphicSize  * 0.5;
            float OrthoTop = m_OrthoGraphicSize  * 0.5;
        
            mProjection = glm::ortho(OrthoLeft,OrthoRight,OrthoBottom,OrthoTop,m_OrthoGraphicNear,m_OrthoGraphicFar);
        }
        
    }
}

