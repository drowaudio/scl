//
// Created by David Rowland on 18/09/2024.
// Modified by the implementation in P0290
//

#pragma once

#include <type_traits>
#include "sync_send.h"


template<typename Type>
class synchronized_value
{
public:
    synchronized_value(const synchronized_value&) = delete;
    synchronized_value &operator=(const synchronized_value&) = delete;

    synchronized_value(synchronized_value&& o) = default;

    template<typename... Args>
    synchronized_value(Args &&... args)
        : val (std::forward<Args> (args)...)
    {}

    template<typename Fn, typename Up, typename... Types>
    friend std::invoke_result_t<Fn, Up&, Types&...> apply (Fn&&, synchronized_value<Up>&,
                                                           synchronized_value<Types>&...);

private:
    std::mutex mutex;
    Type val;
};

template<typename _Fn, typename _Tp, typename... _Types>
inline std::invoke_result_t<_Fn, _Tp &, _Types &...> apply(_Fn &&__f, synchronized_value<_Tp> &__val,
                                                      synchronized_value<_Types> &...__vals) {
    std::scoped_lock __l(__val.mutex, __vals.mutex...);
    return std::__invoke(std::forward<_Fn>(__f), __val.val, __vals.val...);
}

template<typename T>
struct is_send<synchronized_value<T>&> : std::true_type {};

template<typename T>
struct is_sync<synchronized_value<T>> : std::true_type {};


#include <string>
#include <cassert>


synchronized_value<std::string> s;

std::string read_value() {
    return apply([](auto &x) {  return x; }, s);
}

void set_value(std::string const &new_val) {
    apply([&](auto &x) { x = new_val; }, s);
}

void test_single() {
    set_value("new value");
    assert(read_value() == "new value");
}

void test_multi() {
    synchronized_value<int> a(1), b(2), c(3);
    int sum = apply([](auto &...ints) { return (ints++ + ...); }, a, b, c);
    assert(sum == 6);
    auto get = [](int &i) { return i; };
    assert(apply(get, a) == 2);
    assert(apply(get, b) == 3);
    assert(apply(get, c) == 4);
}

