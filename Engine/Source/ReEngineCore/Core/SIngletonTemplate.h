#pragma once
#include <type_traits>

namespace ReEngine
{
    template<typename T>
    class SingletonTemplate
    {
    protected:
        SingletonTemplate() = default;

    public:
        [[nodiscard]] static T& GetInstance()noexcept(std::is_nothrow_constructible<T>::value)
        {
            static T instance;
            return instance;
        }

        virtual ~SingletonTemplate() noexcept = default;
        SingletonTemplate(const SingletonTemplate&) = delete;
        SingletonTemplate& operator=(const SingletonTemplate&) = delete;

    };
}
