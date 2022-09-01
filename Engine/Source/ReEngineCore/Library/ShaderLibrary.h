#pragma once
#include "Library.h"
#include "Renderer/RHI/Shader.h"

namespace ReEngine
{
    //对Library的偏特化
    template<> 
    class Library<Shader> : public LibraryBase<Library,Shader>
    {
    public:
        Library();
    };
}
