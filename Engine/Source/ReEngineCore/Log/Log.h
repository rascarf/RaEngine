#pragma once
#include "spdlog/spdlog.h"

namespace ReEngine
{
    class Log
    {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
        {
            return CoreLogger;
        }

        inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
        {
            return ClientLogger;
        }

    private:
        static std::shared_ptr<spdlog::logger> CoreLogger;
        static std::shared_ptr<spdlog::logger> ClientLogger;
    };

    // class Error
    // {
    // public:
    //     static void Init();
    //
    //     inline static std::shared_ptr<spdlog::logger>& GetCoreLogger()
    //     {
    //         return CoreError;
    //     }
    //
    //     inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
    //     {
    //         return ClientError;
    //     }
    //
    // private:
    //     static std::shared_ptr<spdlog::logger> CoreError;
    //     static std::shared_ptr<spdlog::logger> ClientError;
    // };
}

//Log Macros

#define RE_CORE_TRACE(...) ::ReEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RE_CORE_ERROR(...) ::ReEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RE_CORE_WARN(...) ::ReEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RE_CORE_INFO(...) ::ReEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RE_CORE_FATAL(...) ::ReEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define RE_TRACE(...) ::ReEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RE_INFO(...) ::ReEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define RE_ERROR(...) ::ReEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define RE_WARN(...) ::ReEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RE_FATAL(...) ::ReEngine::Log::GetClientLogger()->fatal(__VA_ARGS__)

