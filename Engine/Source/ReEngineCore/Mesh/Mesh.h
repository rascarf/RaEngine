#pragma once
#include "ReEngineCore/Core/Core.h"

namespace ReEngine
{
    class Mesh
    {
    public:

        virtual std::vector<float>& GetVertexs(){return vertices;}
        virtual std::vector<uint16>& GetIndices(){return indices;}
        
    protected:
        std::vector<float> vertices;
        std::vector<uint16> indices;
    };
}

