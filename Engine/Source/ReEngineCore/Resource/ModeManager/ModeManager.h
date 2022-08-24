#pragma once
#include "Core/SIngletonTemplate.h"

namespace ReEngine
{
    enum class EditMode
    {
        Select = 0,
        Landscape = 1,
        Foliage = 2,
        BrushEditing = 3,
        MeshPaint = 4
    };
    
    enum class SceneMode
    {
        None = 0,
        EnvironmentHdr = 1,
        SkyBox = 2,
    };

    enum class PhysicsDebugDrawModeFlag
    {
        ReEngine_NoDebug = 0,
        ReEngine_DrawWireframe = 1,
        ReEngine_DrawAabb = 2,
        ReEngine_DrawFeaturesText = 4,
        ReEngine_DrawContactPoints = 8,
        ReEngine_NoDeactivation = 16,
        ReEngine_NoHelpText = 32,
        ReEngine_DrawText = 64,
        ReEngine_ProfileTimings = 128,
        ReEngine_EnableSatComparison = 256,
        ReEngine_DisableBulletLCP = 512,
        ReEngine_EnableCCD = 1024,
        ReEngine_DrawConstraints = (1 << 11),
        ReEngine_DrawConstraintLimits = (1 << 12),
        ReEngine_FastWireframe = (1 << 13),
        ReEngine_DrawNormals = (1 << 14),
        ReEngine_DrawFrames = (1 << 15),
        ReEngine_MAX_DEBUG_DRAW_MODE
    };

    class ModeManager : public SingletonTemplate<ModeManager>
    {
    public:
        static bool IsEditMode(){return ModeManager::GetInstance().bEditState;}
        static void ChangeState(){ModeManager::GetInstance().bEditState = !ModeManager::GetInstance().bEditState;}

    public:
        static int b3DMode;  // 0 for 2D, 1 for 3D
        static bool bHdrUse;
        static SceneMode mSceneMode;
        static bool bShowPhysicsColliders;
        static PhysicsDebugDrawModeFlag mPhysicsDebugDrawModeFlag;
    private:
        static bool bEditState;
        static EditMode mEditMode;
    };
}
