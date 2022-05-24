#pragma once

#ifdef RE_PLATFORM_WINDOWS
    #ifdef RE_BUILD_DLL
        #define RE_API __declspec(dllexport)
    #else
        #define RE_API __declspec(dllimport)
    #endif
#else

#endif

//×óÒÆ
#define BIT(x) (1 << x)

// #define HZ_BIND_EVENT_FN(fn) std::bind(&##fn, this, std::placeholders::_1)
#define RE_BIND_EVENT_FN(EventType,CallBackFunc) [&](std::shared_ptr<EventType> e) {return CallBackFunc(e); }