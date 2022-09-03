﻿#include "OpenGLTexture.h"

#include "stb_image.h"
#include "glad/glad.h"

namespace ReEngine
{
    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
        :mWidth(width),mHeight(height)
    {
        mInternalFormat = GL_RGBA8;
        mDataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D,1,&mRendererID);
        glTextureStorage2D(mRendererID,1,mInternalFormat,mWidth,mHeight);

        glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path& path)
        :mPath(path)
    {
        int Width,Height,Channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = stbi_load(path.string().c_str(), &Width, &Height, &Channels, 0);

        if(data)
        {
            mIsLoaded = true;

            mWidth = Width;
            mHeight = Height;
            
            GLenum internalFormat = 0, dataFormat = 0;
            if (Channels == 4)
            {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (Channels == 3)
            {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }
            else if (Channels == 1)
            {
                internalFormat = GL_RGB;
                dataFormat = GL_UNSIGNED_BYTE;
            }

            mInternalFormat = internalFormat;
            mDataFormat = dataFormat;
            

            glCreateTextures(GL_TEXTURE_2D, 1, &mRendererID);
            glTextureStorage2D(mRendererID, 1, internalFormat, mWidth, mHeight);

            glTextureParameteri(mRendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(mRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(mRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
            glTextureSubImage2D(mRendererID, 0, 0, 0, mWidth, mHeight, dataFormat, type, data);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            stbi_image_free(data);
        }
        else
        {
            throw "Load Texture Failed!";
        }
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1,&mRendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = mDataFormat == GL_RGBA ? 4 : 3;  // bytes per pixel
        RE_CORE_ASSERT(size == mWidth * mHeight * bpp, "Data must be entire texture!");
        glTextureSubImage2D(mRendererID, 0, 0, 0, mWidth, mHeight, mDataFormat, GL_UNSIGNED_BYTE, data);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, mRendererID);
    }

    void OpenGLTexture2D::UnBind() const
    {
        glBindTexture(GL_TEXTURE, 0);
    }

    OpenGLTexture3D::OpenGLTexture3D(uint32_t width, uint32_t height)
    {
        glGenTextures(1, &mRendererID);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mRendererID);
    }

    OpenGLTexture3D::OpenGLTexture3D(uint32_t rendererID, uint32_t width, uint32_t height)
        :  mRendererID(rendererID), mWidth(width), mHeight(height)
    {
    }

    OpenGLTexture3D::~OpenGLTexture3D()
    {
        glDeleteTextures(1, &mRendererID);
    }

    void OpenGLTexture3D::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mRendererID);
    }

    void OpenGLTexture3D::UnBind() const
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    // -----------CubeMap-------------------
    OpenGLCubeMapTexture::OpenGLCubeMapTexture()
    {
        glGenTextures(1, &mRendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);

        mWidth = 512;
        mHeight = 512;

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(uint32_t width, uint32_t height)
        :mWidth(width), mHeight(height)
    {
        glGenTextures(1, &mRendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    OpenGLCubeMapTexture::OpenGLCubeMapTexture(std::vector<std::string>& paths)
        :mPaths(paths)
    {
        glGenTextures(1, &mRendererID);
        OpenGLCubeMapTexture::Generate();
    }

    OpenGLCubeMapTexture::~OpenGLCubeMapTexture()
    {
        glDeleteTextures(1, &mRendererID);
    }

    void OpenGLCubeMapTexture::SetFace(FaceTarget faceIndex, const std::string& path)
    {
        mPaths[(uint32_t)faceIndex] = path;
    }

    void OpenGLCubeMapTexture::GenerateMipmap()
    {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    void OpenGLCubeMapTexture::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);
    }

    void OpenGLCubeMapTexture::UnBind() const
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void OpenGLCubeMapTexture::Generate()
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID);

        int width = 0;
        int height = 0;
        int nrChannels = 0;
        stbi_set_flip_vertically_on_load(false);
        for (unsigned int i = 0; i < mPaths.size(); i++)
        {
            //todo 这里修改成获取绝对路径
            unsigned char* data = stbi_load(mPaths[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                RE_ERROR("Cubemap don't loaded correctly!");
                stbi_image_free(data);
            }
        }

        mWidth = width;
        mHeight = height;

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }


    
    
}
