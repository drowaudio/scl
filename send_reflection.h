//
// Created by David Rowland on 17/09/2024.
//

#include <type_traits>
#include <atomic>

//================================================================================
// Sync Trait: Can be shared between threads
//================================================================================
// In general, no types are sync in C++ because we can't garentee const
// references don't have non-const aliases.
// The only type garantueed by the standard to be data-race free is std::atomic

template<typename T>
struct is_sync : std::false_type {};

template<typename T>
struct is_sync<std::atomic<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_sync_v = is_sync<T>::value;

template<typename... Args>
concept sync = (is_sync<Args>::value && ...);


//================================================================================
#include <string>

static_assert(! is_sync_v<int>);
static_assert(! is_sync_v<int&>);
static_assert(! is_sync_v<const int&>);
static_assert(! is_sync_v<std::string&>);
static_assert(! is_sync_v<const std::string&>);
static_assert(is_sync_v<std::atomic<int>>);



//================================================================================
// Sync Trait: Can be shared between threads
//================================================================================
#if __has_include (<experimental/meta>)

#include <iostream>
#include <memory>
#include <string>
#include <ranges>
#include <algorithm>
#include <experimental/meta>

using std::meta::info;


consteval auto is_send_type (std::meta::info type) -> bool
{
    type = remove_cv (type);

    // Non-member function pointers
    if (is_pointer_type (type)
        && is_function_type (remove_pointer (type))
        && ! is_member_function_pointer_type (type))
       return true;

    // lvalue refs and pointers
    if (is_lvalue_reference_type (type)
        || is_pointer_type (remove_extent (type))
        || is_pointer_type (remove_reference (type)))
       return false;

    // POD built-in types
    if (is_arithmetic_type (type))
        return true;

    // Recursive class/struct/lambda members
    if (is_class_type (type))
        return std::ranges::all_of(nonstatic_data_members_of(type),
                                   [](std::meta::info d)
                                   {
                                       return is_send_type (type_of(d));
                                   });

    // Construct from rvalue ref
    if (is_rvalue_reference_type (type)
        && is_constructible_type (type, { remove_reference (type) }))
       return true;

    return false;
}

template<typename T>
consteval auto is_send() -> bool
{
    if (is_send_type (^^T))
        return true;

    return is_sync_v<T>;
}

template<typename T>
inline constexpr bool is_send_v = is_send<T>();

template<typename T>
concept send = is_send_v<T>;

//======================================================
// Tests
//======================================================
static_assert(is_send_v<const int>);
static_assert(is_send_v<int>);
static_assert(is_send_v<int&&>);
static_assert(! is_send_v<int&>);
static_assert(! is_send_v<int*&>);
static_assert(! is_send_v<int*&&>);
static_assert(! is_send_v<const int&>);
static_assert(! is_send_v<const int*&>);
static_assert(! is_send_v<std::string&>);
static_assert(! is_send_v<const std::string&>);
static_assert(! is_send_v<std::string*&>);
static_assert(! is_send_v<const std::string*&>);

using fn = void (*)(int);
static_assert(is_pointer_type (^^fn));
static_assert(is_function_type (remove_pointer (^^fn)));
static_assert(! is_member_function_pointer_type (^^fn));
static_assert(is_send_v<fn>);


struct node
{
    node* prev;
    node* next;
};
static_assert(! is_send_v<node>);

static_assert(is_send_v<decltype([] {})>);

void lambda_test()
{
    [[maybe_unused]] auto non_capturing = [] (int) {};
    static_assert(is_send_v<decltype(non_capturing)>);

    int i = 0;
    [[maybe_unused]] auto val_capturing = [i] (int) {};
    static_assert(is_send_v<decltype(val_capturing)>);

    [[maybe_unused]] auto ref_capturing = [&i] (int) {};
    static_assert(! is_send_v<decltype(ref_capturing)>);

    struct type
    {
        type()
        {
            auto n = std::make_shared<node>();

            [[maybe_unused]] auto this_capturing = [this] { run(); };
            static_assert(! is_send_v<decltype(this_capturing)>);

            [[maybe_unused]] auto this_n_capturing = [this, n] { run(); };
            static_assert(! is_send_v<decltype(this_n_capturing)>);

            [[maybe_unused]] auto n_ref_capturing = [&n] {};
            static_assert(! is_send_v<decltype(n_ref_capturing)>);

            [[maybe_unused]] auto n_val_capturing = [n] {};
            static_assert(! is_send_v<decltype(n_val_capturing)>);
        }

        void run()
        {
        }
    };
}

#else

// Fallback to single level with no reflection

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
                    (! (std::is_lvalue_reference_v<T>
                        || (std::is_pointer_v<std::remove_extent_t<T>>  // This shouldn't include non-member function pointers
                        || is_lambda_v<T>))
                    &&
                    (std::is_move_constructible_v<T>
                     || (is_function_pointer_v<std::decay_t<T>>
                         && ! std::is_member_function_pointer_v<T>)
                     || is_sync_v<T>)>
{};

#endif //__has_include (<experimental/meta>)