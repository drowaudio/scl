#include <functional>
#include <memory>
#include <print>
#include <ranges>
#include <scl/safe_thread.h>

using namespace std::literals;

int main()
{
    std::vector<scl::thread> threads { };

    {
        // Launch all threads.
        const int num_threads = 15;
        int mol = 42;

        for (int i : std::views::iota (0, num_threads))
        {
            static_assert(! is_send_v<decltype([&mol] {})>);
            threads.push_back (scl::thread ([&mol] { std::print ("Hello safe_thread {}", mol); }));
        }
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}