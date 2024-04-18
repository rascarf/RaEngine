#include "AsyncLoader.h"

#include "Core/Application.h"

namespace ReEngine
{
    AsyncLoader::~AsyncLoader()
    {
    }

    void AsyncLoader::Init(AsyncLoaderMainTask* InMainTask)
    {
        OwnerTask = InMainTask;
    }

    void AsyncLoader::Update()
    {
        RE_INFO("Test from {0}",OwnerTask->threadNum);
    }

    void AsyncLoader::ShutDown()
    {
    }

    void AsyncLoader::RequestTexture()
    {
    }

    void AsyncLoader::RequestBufferUpload()
    {
    }

    void AsyncLoader::RequestBufferCopy()
    {
    }
}

