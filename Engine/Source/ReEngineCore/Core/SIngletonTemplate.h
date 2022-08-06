#pragma once
#include <type_traits>

namespace ReEngine
{
    template<template T>
    class SingletonTemplate
    {
    protected:
        SingletonTemplate() = default;

    public:
        [[nodiscard]] static T& GetInstance()noexcept(std::is_nothrow_constructible::value)
        {
            static T instance;
            return instance;
        }

        virtual ~SingletonTemplate() noexcept = default;
        PublicSingleton(const PublicSingleton&) = delete;
        PublicSingleton& operator=(const PublicSingleton&) = delete;

    };
}
