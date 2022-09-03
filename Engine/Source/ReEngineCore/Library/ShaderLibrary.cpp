#include "ShaderLibrary.h"

namespace ReEngine
{
    Library<Shader>::Library()
    {
        //TODO 改成编译好的字节码，直接Load上来
        Add("TextureShader",ReEngine::Shader::Create(std::string("shaders/texture.glsl")));
        Add("FlatShader",ReEngine::Shader::Create(std::string("shaders/FlatColor.glsl")));
        
    }
}
