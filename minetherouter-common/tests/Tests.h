#pragma once

#include <iostream>
#include <type_traits>
#include <cstdlib>

template<typename T>
auto ToPrintableValue(const T& value)
{
    if constexpr (std::is_enum_v<T>) {
        return static_cast<std::underlying_type_t<T>>(value);
    }
    else {
        return value;
    }
}

#define ASSERT_EQ(a,b) do { \
    auto _va = (a); \
    auto _vb = (b); \
    if (_va != _vb) { \
        std::cerr << "ASSERT_EQ failed at " << __FILE__ << ":" << __LINE__ << "\n" \
                  << " expected: " << #a << " == " << #b << "\n" \
                  << " actual:   " << ToPrintableValue(_va) << " != " << ToPrintableValue(_vb) << "\n"; \
        std::exit(1); \
    } \
} while(0)

#define RUN_TEST(name) do { \
    std::cout << "[ RUN      ] " << #name << "\n"; \
    name(); \
    std::cout << "[     OK   ] " << #name << "\n"; \
    std::cout << "------------" << "\n"; \
} while(0)