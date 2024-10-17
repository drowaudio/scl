//
// Created by David Rowland on 17/09/2024.
//

#include "type_traits.h"
#include <memory>
#include <print>
#include <string>
#include <type_traits>

struct FailureStruct {
    int& i;
    void operator()();
};

static_assert (std::is_class_v<FailureStruct>);
static_assert (std::is_copy_constructible_v<FailureStruct>);
static_assert (std::is_move_constructible_v<FailureStruct>);
static_assert (! std::is_copy_assignable_v<FailureStruct>);
static_assert (! std::is_move_assignable_v<FailureStruct>);

struct PassStruct {
    int i;
    void operator()();
    PassStruct(PassStruct&) = delete;
};

static_assert (std::is_class_v<PassStruct>);
static_assert (std::is_copy_assignable_v<PassStruct>);
static_assert (std::is_move_assignable_v<PassStruct>);
// static_assert (std::is_copy_constructible_v<PassStruct>);
// static_assert (std::is_move_constructible_v<PassStruct>);

// struct PrivateStruct {
//     PrivateStruct (int& in)
//         : i (in) {}
//
//     void operator()() {
//         std::println ("{}", i);
//     }
// private:
//     int i;
// };

class PrivateStruct
{
public:
    inline /*constexpr */ void operator()() const
    {
        std::println ("{}", i);
    }

private:
    int i;

public:
    PrivateStruct(int & _i)
    : i{_i}
    {}

};

static_assert (std::is_class_v<PrivateStruct>);
static_assert (std::is_copy_assignable_v<PrivateStruct>);
static_assert (std::is_move_assignable_v<PrivateStruct>);
static_assert (std::is_copy_constructible_v<PrivateStruct>);
static_assert (std::is_move_constructible_v<PrivateStruct>);

struct MovableStruct {
    std::unique_ptr<int> i;
    void operator()();
};

static_assert (std::is_class_v<MovableStruct>);
static_assert (! std::is_copy_assignable_v<MovableStruct>);
static_assert (std::is_move_assignable_v<MovableStruct>);
static_assert (! std::is_copy_constructible_v<MovableStruct>);
static_assert (std::is_move_constructible_v<MovableStruct>);

template<typename T>
struct is_sendable_class : std::integral_constant<
                    bool,
                    (std::is_class_v<T>
                     && (std::is_copy_assignable_v<T>
                         || std::is_move_assignable_v<T>))>
{};

template<typename T>
inline constexpr bool is_sendable_class_v = is_sendable_class<T>::value;

static_assert (is_sendable_class_v<PassStruct>);
static_assert (is_sendable_class_v<MovableStruct>);
static_assert (! is_sendable_class_v<FailureStruct>);

static_assert (std::is_class_v<PassStruct> && std::is_copy_assignable_v<PassStruct>);
static_assert (std::is_class_v<FailureStruct> && ! std::is_copy_assignable_v<FailureStruct>);


static_assert (! is_lambda_v<int>);
static_assert (! is_lambda_v<std::string>);

void test()
{
    // Sendable: plain lambda (decays to function pointer)
    {
        auto lambda = []() {};

        static_assert (is_sendable_class_v<decltype(lambda)>);
        static_assert (is_lambda_v<decltype(lambda)>);
        static_assert (std::is_class_v<decltype(lambda)>);
    }

    // Sendable: copyable lambda
    {
        int i = 42;
        auto lambda = [i]() { std::println ("{}", i); };

        // static_assert (std::is_copy_assignable_v<decltype(lambda)>);
        // static_assert (std::is_copy_constructible_v<decltype(lambda)>);
        // static_assert (is_sendable_class_v<decltype(lambda)>); //ddd Should pass
        static_assert (is_lambda_v<decltype(lambda)>);
        static_assert (std::is_class_v<decltype(lambda)>);
    }

    // Not Sendable: reference lambda
    {
        int i = 42;
        auto lambda = [&i]() { std::println ("{}", i); };

        static_assert (std::is_copy_constructible_v<decltype(lambda)>);
        static_assert (! is_sendable_class_v<decltype(lambda)>);
        static_assert (is_lambda_v<decltype(lambda)>);
        static_assert (std::is_class_v<decltype(lambda)>);
    }

    {
        struct TestCallable
        {
            int operator()()
            {
                return 42;
            }
        };

        static_assert (is_lambda_v<TestCallable>);
        static_assert (std::is_class_v<TestCallable>);
    }

    {
        struct TestCallable
        {
            int operator()(int i)
            {
                return i;
            }
        };

        static_assert (is_lambda_v<TestCallable>);
        static_assert (std::is_class_v<TestCallable>);
    }
}
