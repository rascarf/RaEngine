#pragma once
#include "Core/Core.h"
#include "Mesh.h"
#include "Core/MathDefine.h"

namespace ReEngine
{
    // 仅仅给线绘制模式
    class LineSphere : public Mesh
    {
    public:
        LineSphere(int32 Sphslices,float Scale)
        {
            int32 Count = 0;
            int32 Slics = Sphslices;
            int32 Stacks = Slics;
            
            vertices.resize((Slics + 1) * Stacks * 3 * 2);

            float ds = 1.0f / Sphslices;
            float dt = 1.0f / Sphslices;
            float t  = 1.0;
            float drho   = PI / Stacks;
            float dtheta = 2.0 * PI / Slics;

            for (int32 i= 0; i < Stacks; ++i) 
            {
                float rho = i * drho;
                float s   = 0.0;
                for (int32 j = 0; j<=Slics; ++j) {
                    float theta = (j == Slics) ? 0.0f : j * dtheta;
                    float x = -sin(theta) * sin(rho) * Scale;
                    float z =  cos(theta) * sin(rho) * Scale;
                    float y = -cos(rho) * Scale;

                    vertices[Count + 0] = x;
                    vertices[Count + 1] = y;
                    vertices[Count + 2] = z;
                    Count += 3;

                    x = -sin(theta) * sin(rho+drho) * Scale;
                    z =  cos(theta) * sin(rho+drho) * Scale;
                    y = -cos(rho+drho) * Scale;

                    vertices[Count + 0] = x;
                    vertices[Count + 1] = y;
                    vertices[Count + 2] = z;
                    Count += 3;

                    s += ds;
                }
                
                t -= dt;
            }
        }
    };    
}

