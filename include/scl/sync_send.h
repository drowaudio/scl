//
// Created by David Rowland on 17/09/2024.
//

#pragma once

#include <type_traits>
#include <mutex>
#include <atomic>
#include <memory>

namespace scl {
/**
    A wrapper that can be used around pointers to allow the this pointer to be passed.
    There's nothing special about this, it's just a dump pointer but means we can
    explicitly allow it when we get reflection and makes it obvious in calling
    code that the intent is to share it.
 */
template<typename T>
class unchecked_pointer
{
public:
    unchecked_pointer (nullptr_t) = delete;

    unchecked_pointer (T* p)
        : pointer (p) {
    }

    unchecked_pointer (T& p)
        : pointer (*p) {
    }

    T& operator*() const {
        return *pointer;
    }

    T* operator->() const {
        return pointer;
    }

private:
    T* pointer;
};


//================================================================================
// Some additional traits used in sync and send below
//================================================================================
template <typename T>
struct is_const_reference : std::false_type {};

// Specialization for const T&
template <typename T>
struct is_const_reference<const T&> : std::true_type {};

// Helper variable template for easier usage
template <typename T>
inline constexpr bool is_const_reference_v = is_const_reference<T>::value;

template<typename T>
struct is_function_pointer
    : std::integral_constant<bool,
          std::is_pointer_v<T> && std::is_function_v<typename std::remove_pointer<T>::type>>
{};

template<typename T>
inline constexpr bool is_function_pointer_v = is_function_pointer<T>::value;


// Helper for detecting if a type is callable (i.e., has an operator())
template <typename T>
struct is_callable {
private:
    template <typename U>
    static auto test(int) -> decltype(&U::operator(), std::true_type());

    // Fallback for non-callable objects
    template <typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// Lambda detection
template <typename T>
struct is_lambda : std::integral_constant<bool, is_callable<T>::value && !std::is_function<T>::value> {};

template<typename T>
inline constexpr bool is_lambda_v = is_lambda<T>::value;


//================================================================================
// Sync Trait: Can be shared between threads
//================================================================================
// In general, no types are sync in C++ because we can't garentee const
// references don't have non-const aliases.
// The only type garantueed by the standard to be data-race free is std::atomic

// template<typename T>
// struct is_sync : std::integral_constant<
//                     bool,
//                     is_const_reference_v<T>> {};
template<typename T>
struct is_sync : std::false_type {};

template<typename T>
struct is_sync<std::atomic<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_sync_v = is_sync<T>::value;

template<typename... Args>
concept sync = (is_sync<Args>::value && ...);


//================================================================================
// Send Trait: Can be moved between threads
//================================================================================
// Send Trait: Checks if a type can be safely passed between threads (is trivially movable)
// In general this is:
// - If the type is NOT:
//    - An lvalue reference (const or non-const, this is stricter than Rust/Circle)
//    - OR any kind of pointer
//    - OR any kind of lambda (callable object)
// - AND is:
//    - A POD type
//    - OR a move-constructable type
//    - OR a pointer to a non-member (global) function
//    - OR the type is sync
template <typename T>
struct is_send : std::integral_constant<
                    bool,
                    (! ((std::is_lvalue_reference_v<T>
                         && ! std::is_function_v<std::remove_cvref_t<T>>)
                        || (std::is_pointer_v<std::remove_extent_t<std::decay_t<T>>>
                            && ! is_function_pointer_v<std::decay_t<T>>)  // This shouldn't include non-member function pointers
                        || is_lambda_v<T>))
                    &&
                    (std::is_move_constructible_v<T>
                     || (is_function_pointer_v<std::decay_t<T>>
                         && ! std::is_member_function_pointer_v<std::decay_t<T>>)
                     || std::is_function_v<std::decay_t<T>>
                     || is_sync_v<T>)>
{};

// Test doing function first
// template <typename T>
// struct is_send : std::integral_constant<
//                     bool,
//                     (std::is_function_v<std::remove_cvref_t<T>>
//                      && ! std::is_member_function_pointer_v<std::decay_t<T>>)
//                     &&
//                         (! is_lambda_v<T>
//                         || (std::is_rvalue_reference_v<std::remove_extent_t<std::remove_cv_t<T>>>
//                             && std::is_move_constructible_v<T>)
//                         || is_sync_v<T>)>
//
// {};

// template<typename T> struct is_send<T*&>        : std::false_type {};
// template<typename T> struct is_send<T*&&>       : std::false_type {};
// template<typename T> struct is_send<const T*&>  : std::false_type {};
// template<typename T> struct is_send<const T*&&> : std::false_type {};
//
// template<typename T> struct is_send<T*>         : std::false_type {};
// template<typename T> struct is_send<const T*>   : std::false_type {};


// template<typename T>
//     requires (is_function_pointer_v<std::decay_t<T>>
//               && ! std::is_member_function_pointer_v<T>)
// struct is_send<T> : std::true_type {};



// template<typename T>
//     requires std::is_pointer_v<T> && std::is_object_v<T>
// struct is_send<T> : std::integral_constant<
//                     bool,
//                     ! (std::is_pointer_v<std::remove_cvref_t<T>>
//                         || std::is_object_v<std::remove_cvref_t<T>>)>
// {};

//
// template<typename T>
// struct is_send<const T*> : std::negation<std::is_member_function_pointer<T>> {};

// template<typename T>
// struct is_send<T&> : std::false_type {};
//
// template<typename T>
// struct is_send<const T&> : std::false_type {};

// Additionally, shared_ptrs to types that are sync are send
// template<typename T>
// struct is_send<std::shared_ptr<T>> : std::integral_constant<
//                                         bool,
//                                         is_sync_v<T>>
// {};

template<sync T>
struct is_send<std::shared_ptr<T>> : std::true_type
{};

template<typename T>
inline constexpr bool is_send_v = is_send<T>::value;

template<typename T>
concept send = is_send<T>::value;


//======================================================
// Tests
//======================================================
static_assert(is_send_v<const int>);
static_assert(is_send_v<int>);
static_assert(is_send_v<int&&>);
static_assert(! is_send_v<int&>);
static_assert(! is_send_v<int*&>);
static_assert(! is_send_v<const int&>);
static_assert(! is_send_v<const int*&>);
static_assert(! is_send_v<std::string&>);
static_assert(! is_send_v<const std::string&>);
static_assert(! is_send_v<std::string*&>);
static_assert(! is_send_v<const std::string*&>);
static_assert(is_send_v<void (*)(int)>);
static_assert(is_send_v<void (&)(int)>);

static_assert(! is_sync_v<int>);
static_assert(! is_sync_v<int&>);
static_assert(! is_sync_v<const int&>);
static_assert(! is_sync_v<std::string&>);
static_assert(! is_sync_v<const std::string&>);
static_assert(is_sync_v<std::atomic<int>>);

}