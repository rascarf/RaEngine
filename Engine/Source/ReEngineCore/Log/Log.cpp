#include "Log/Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ReEngine
{
    std::shared_ptr<spdlog::logger>Log::CoreLogger;
    std::shared_ptr<spdlog::logger>Log::ClientLogger;

    // std::shared_ptr<spdlog::logger>Error:: CoreError;
    // std::shared_ptr<spdlog::logger>Error:: ClientError;

    void Log::Init()
    {
        spdlog::set_pattern(
            "%^[%T] %n:%v%$"
        );

        CoreLogger = spdlog::stdout_color_mt("ReEngine");
        CoreLogger->set_level(spdlog::level::err);

        ClientLogger = spdlog::stdout_color_mt("App");
        ClientLogger->set_level(spdlog::level::trace);
    }

    // void Error::Init()
    // {
    //     CoreError = spdlog::stdout_color_mt("ReEngine");
    //     CoreError->set_level(spdlog::level::err);
    //
    //     ClientError = spdlog::stdout_color_mt("App");
    //     ClientError->set_level(spdlog::level::err);
    // }
}
