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
        // RE_INFO("Test from {0}",OwnerTask->threadNum);

        if(FileLoadRequests.GetSize() > 0)
        {
            auto FileRequest = FileLoadRequests.Back();
            FileLoadRequests.Pop_Back();

            RE_INFO("Path: {0}",FileRequest.Path);
        }
    }

    void AsyncLoader::ShutDown()
    {
    }

    void AsyncLoader::RequestTexture(std::string path)
    {
        FileLoadRequest Request;
        Request.Path = path;

        FileLoadRequests.Emplace_Back(Request);
    }

    void AsyncLoader::RequestBufferUpload()
    {
    }

    void AsyncLoader::RequestBufferCopy()
    {
    }
}

