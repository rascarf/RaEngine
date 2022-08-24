#pragma once

#include <memory>
#include "Assert.h"

#ifdef RE_PLATFORM_WINDOWS
    #ifdef RE_BUILD_DLL
        #define RE_API __declspec(dllexport)
    #else
        #define RE_API __declspec(dllimport)
    #endif
#else

#endif

#define BIT(x) (1 << x)
#define RE_BIND_EVENT_FN(EventType,CallBackFunc) [&](std::shared_ptr<EventType> e) {return CallBackFunc(e); }
#define RE_XSTRINGIFY_MACRO(x) RE_STRINGIFY_MACRO(x)
#define RE_STRINGIFY_MACRO(x) #x

namespace ReEngine
{
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}
