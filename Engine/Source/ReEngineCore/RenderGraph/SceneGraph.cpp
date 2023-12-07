#include "SceneGraph.h"

namespace ReEngine
{
    void SceneGraph::Init(uint32 NumNodes)
    {
        NodesHierarchy.resize(NumNodes);
        LocalMatrices.resize(NumNodes);
        WorldMatrices.resize(NumNodes);
        UpdatedNodes.resize(NumNodes);
    }

    void SceneGraph::ShutDown()
    {
        NodesHierarchy.empty();
        LocalMatrices.empty();
        WorldMatrices.empty();
        UpdatedNodes.empty();
    }

    void SceneGraph::Resize(uint32 NumNodes)
    {
        NodesHierarchy.resize(NumNodes);
        LocalMatrices.resize(NumNodes);
        WorldMatrices.resize(NumNodes);
        UpdatedNodes.resize(NumNodes);

        memset( NodesHierarchy.data(), 0, NumNodes * 4 );
        for ( uint32 i = 0; i < NumNodes; ++i )
        {
            NodesHierarchy[ i ].Parent = -1;
        }
    }

    void SceneGraph::UpdateMatrices()
    {
        uint32 max_level = 0;
        for ( uint32 i = 0; i < NodesHierarchy.size(); ++i )
        {
            max_level = std::max(max_level, (uint32)NodesHierarchy[ i ].Level );
        }
        
        uint32 current_level = 0;
        uint32 nodes_visited = 0;

        while ( current_level <= max_level )
        {

            for ( uint32 i = 0; i < NodesHierarchy.size(); ++i ) {

                if ( NodesHierarchy[ i ].Level != current_level )
                {
                    continue;
                }

                if ( UpdatedNodes[i] == false )
                {
                    continue;
                }

                UpdatedNodes[i] == false;

                if ( NodesHierarchy[ i ].Parent == -1 )
                {
                    WorldMatrices[ i ] = LocalMatrices[ i ];
                }
                else
                {
                    const glm::mat4& parent_matrix = WorldMatrices[ NodesHierarchy[ i ].Parent ];
                    WorldMatrices[ i ] =  parent_matrix *  LocalMatrices[ i ];
                }

                ++nodes_visited;
            }

            ++current_level;
        }
    }

    void SceneGraph::SetHierarchy(uint32 NodeIndex, uint32 ParentIndex, uint32 Level)
    {
        UpdatedNodes[NodeIndex] = true;
        NodesHierarchy[ NodeIndex ].Parent = ParentIndex;
        NodesHierarchy[ NodeIndex ].Level = Level;

        SortUpdateOrder = true;
    }

    void SceneGraph::SetLocalMatrix(uint32 Node_Index, const glm::mat4 LocalMatrix)
    {
        // Mark node as updated
        UpdatedNodes[Node_Index] = true;
        LocalMatrices[ Node_Index ] = LocalMatrix;
    }
}
