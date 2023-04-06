#pragma once
#include "Core/Core.h"
#include "Core/Timestep.h"


namespace ReEngine
{
  class WindowProperty;
    
  class GraphicsContext
  {
  public:
      virtual void Init() = 0;
      virtual void SwapBuffers(Timestep ts) = 0;
      virtual void Close(){}
      virtual ~GraphicsContext(){}
      
      static Ref<GraphicsContext> Create(void* Window,const WindowProperty* WindowProperty);
      
  };
};