#include "Layer/Layer.h"
#include "Layer/LayerStack.h"


ReEngine::LayerStack::LayerStack()
{
}

ReEngine::LayerStack::~LayerStack()
{
    for(auto& Layer : mLayerStack)
    {
        delete Layer;
    }
}

void ReEngine::LayerStack::PushLayer(Layer* InLayer)
{
    mLayerStack.emplace(mLayerStack.begin() + LayerInsertIndex, InLayer);
}

void ReEngine::LayerStack::PopLayer(Layer* InLayer)
{
    auto it = std::find(mLayerStack.begin(), mLayerStack.end(), InLayer);
    if(it != mLayerStack.end())
    {
        mLayerStack.erase(it);
        LayerInsertIndex--;
    }
}

void ReEngine::LayerStack::PushOverlay(Layer* Overlay)
{
    mLayerStack.emplace_back(Overlay);
}

void ReEngine::LayerStack::PopOverlay(Layer* Overlay)
{
    auto it = std::find(mLayerStack.begin(), mLayerStack.end(), Overlay);
    if (it != mLayerStack.end())
        mLayerStack.erase(it);
}
