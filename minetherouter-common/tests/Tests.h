#pragma once

#define ASSERT_EQ(a,b) do { \
    auto _va = (a); \
    auto _vb = (b); \
    if (_va != _vb) { \
        std::cerr << "ASSERT_EQ failed at " << __FILE__ << ":" << __LINE__ << "\n" \
                  << " expected: " << #a << " == " << #b << "\n" \
                  << " actual:   " << _va << " != " << _vb << "\n"; \
        std::exit(1); \
    } \
} while(0)

#define RUN_TEST(name) do { \
    std::cout << "[ RUN      ]" << #name << "\n"; \
    name(); \
    std::cout << "[     OK   ]" << #name << "\n"; \
    std::cout << "------------" << "\n"; \
} while(0)