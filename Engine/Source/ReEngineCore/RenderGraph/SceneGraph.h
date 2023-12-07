#pragma once
#include "Core/Core.h"
#include "glm/glm.hpp"

namespace ReEngine
{
    struct Hierarchy
    {
        int32 Parent : 24;
        int32 Level : 8;
    };
    
    // Node的层级管理
    struct SceneGraph
    {
        void Init(uint32 NumNodes);
        void ShutDown();

        void Resize(uint32 NumNodes);
        void UpdateMatrices();

        void SetHierarchy(uint32 NodeIndex,uint32 ParentIndex,uint32 Level);
        void SetLocalMatrix(uint32 Node_Index,const glm::mat4 LocalMatrix);

        std::vector<glm::mat4> LocalMatrices;
        std::vector<glm::mat4> WorldMatrices;
        std::vector<Hierarchy> NodesHierarchy;
        std::vector<bool> UpdatedNodes;
        bool SortUpdateOrder = true;
    };
    
}
