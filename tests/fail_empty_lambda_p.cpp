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

        for ([[maybe_unused]] int i : std::views::iota (0, num_threads))
            threads.push_back (scl::thread ([] { std::print ("Hello scl::thread"); }));
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}