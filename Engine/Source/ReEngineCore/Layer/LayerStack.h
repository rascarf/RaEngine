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

 
        void PushLayer(Ref<Layer> InLayer);
        void PopLayer(Ref<Layer> InLayer);
        void PushOverlay(Ref<Layer> Overlay);
        void PopOverlay(Ref<Layer> Overlay);

        inline std::vector<Ref<Layer>>::iterator begin() { return mLayerStack.begin(); }
        inline std::vector<Ref<Layer>>::iterator end() { return mLayerStack.end(); }
    private:
        std::vector<Ref<Layer>>mLayerStack;
        unsigned int  LayerInsertIndex = 0;
    };
}


