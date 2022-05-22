#pragma once
#include "Core/Core.h"
#include <vector>
#include "Layer.h"

namespace ReEngine
{
    class LayerStack
    {
    public:
        LayerStack();
        ~LayerStack();

 
        void PushLayer(ReEngine::Layer* InLayer);
        void PopLayer(ReEngine::Layer* InLayer);
        void PushOverlay(ReEngine::Layer* Overlay);
        void PopOverlay(ReEngine::Layer* Overlay);

        inline std::vector<ReEngine::Layer*>::iterator begin() { return mLayerStack.begin(); }
        inline std::vector<ReEngine::Layer*>::iterator end() { return mLayerStack.end(); }
    private:
        std::vector<ReEngine::Layer*>mLayerStack;
        std::vector<ReEngine::Layer*>::iterator LayerInsertIt;
    };
}


