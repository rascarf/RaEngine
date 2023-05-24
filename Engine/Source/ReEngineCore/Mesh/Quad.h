#pragma once
#include "Mesh.h"
#include "Core/Core.h"

namespace ReEngine
{
    class Quad : public ReEngine::Mesh
    {
    public:
        Quad()
        {
            std::vector<float> verts = {
                -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
            };
            
            std::vector<uint16> ins = {
                0, 1, 2, 0, 2, 3
            };

            vertices = verts;
            indices = ins;
        }
        
        ~Quad()
        {
            vertices.empty();
            indices.empty();
        }
    };
}
