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

        for (int i : std::views::iota (0, num_threads))
        {
            auto ptr = std::make_unique<int> (i);
            threads.push_back (scl::thread ([p = std::move (ptr)] { std::print ("Hello safe_thread {}", *p); }));
        }
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}