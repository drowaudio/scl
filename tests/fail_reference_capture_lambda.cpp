#include <functional>
#include <memory>
#include <print>
#include <ranges>
#include "../safe_thread.h"

using namespace std::literals;

int main()
{
    std::vector<safe_thread> threads { };

    {
        // Launch all threads.
        const int num_threads = 15;
        int mol = 42;

        for (int i : std::views::iota (0, num_threads))
        {
            static_assert(! is_send_v<decltype([&mol] {})>);
            threads.push_back (safe_thread ([&mol] { std::print ("Hello safe_thread {}", mol); }));
        }
    }

    // Join all threads.
    for (safe_thread& t : threads)
        t.join();
}