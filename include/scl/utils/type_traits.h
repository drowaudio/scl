//
// Created by David Rowland on 17/09/2024.
//

#pragma once

#include <type_traits>

// Helper to check if a class has a call operator
template<typename T>
struct is_lambda_impl
{
private:
    // Check if `T` has an operator()
    template<typename U>
    static auto test(U*) -> decltype(&U::operator(), std::true_type{});

    // Fallback for types that don't have `operator()`
    template<typename>
    static std::false_type test(...);

public:
    // Use decltype and std::is_same to create the final trait
    static constexpr bool value = std::is_same_v<decltype(test<T>(nullptr)), std::true_type>;
};

// Type trait to detect lambda types
template<typename T>
struct is_lambda : std::conditional_t<
    std::is_class_v<T>,   // Lambdas are classes
    is_lambda_impl<T>,    // Check if the class has a call operator
    std::false_type       // Non-class types can't be lambdas
> {};

// Helper variable template
template<typename T>
inline constexpr bool is_lambda_v = is_lambda<T>::value;
