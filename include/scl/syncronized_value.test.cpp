#include <string>
#include <cassert>
#include "synchronized_value.h"

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


struct person
{
    std::string get_name() const;
    void set_name (std::string);

    std::string name;
};

template<>
struct is_sync<person> : std::true_type {};

static_assert(is_send_v<person>);

int main()
{
    test_single();
    test_multi();
}