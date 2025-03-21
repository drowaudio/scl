#include <functional>
#include <memory>
#include <print>
#include <scl/safe_thread.h>

using namespace std::literals;

template <typename T>
struct is_send_test : std::integral_constant<
                    bool,
                    (! (std::is_lvalue_reference_v<T>
                        || scl::is_lambda_v<T>))
                    &&
                    (std::is_pod_v<T>
                     || std::is_move_constructible_v<T>
                     || (scl::is_function_pointer_v<std::decay_t<T>>
                         && ! std::is_member_function_pointer_v<T>)
                     || scl::is_sync_v<T>)>
{};

template<typename T> struct is_send_test<T*>        : std::false_type {};
template<typename T> struct is_send_test<const T*>        : std::false_type {};

template<typename T> struct is_send_test<T*&>        : std::false_type {};
template<typename T> struct is_send_test<T*&&>       : std::false_type {};
template<typename T> struct is_send_test<const T*&>  : std::false_type {};
template<typename T> struct is_send_test<const T*&&> : std::false_type {};

template<typename T>
void check (T&&) {
    static_assert (! is_send_test<T>::value);
}

class thread_runner
{
public:
    thread_runner()
    {
        thread = std::make_unique<scl::thread> (std::mem_fn (&thread_runner::run), this);
    }

private:
    std::unique_ptr<scl::thread> thread;

    void run() const
    {
        std::println("PROCESSING...");
        std::this_thread::sleep_for (1s);
        std::println("...DONE");
    }
};

int main() {
    thread_runner r;
}