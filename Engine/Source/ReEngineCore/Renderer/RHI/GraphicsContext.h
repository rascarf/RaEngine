#pragma once
#include "Core/Core.h"


namespace ReEngine
{
  class WindowProperty;
    
  class GraphicsContext
  {
  public:
      virtual void Init() = 0;
      virtual void SwapBuffers() = 0;
      virtual void Close(){}

      static Ref<GraphicsContext> Create(void* Window,const WindowProperty* WindowProperty);
  };
};