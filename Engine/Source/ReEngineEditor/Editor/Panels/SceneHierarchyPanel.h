#pragma once
#include "Core/PCH.h"
#include "Log/Log.h"
#include "ECSFrameWork/Scene.h"
#include "ECSFrameWork/Entity/Entity.h"

namespace ReEngine
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel();
        SceneHierarchyPanel(const Ref<Scene>& Scene);

        void SetContext(const Ref<Scene>& scene);

        void OnImGuiRender();

    private:
        void DrawEntityNode(Entity entity);

        void DrawComponent(Entity SelectContext);
    private:

        Ref<Scene> mContext;
        Entity mSelectContext;
        
    };
}
