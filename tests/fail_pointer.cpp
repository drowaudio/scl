#include <functional>
#include <memory>
#include <print>
#include <ranges>
#include <scl/safe_thread.h>

using namespace std::literals;

void entry_point (int* tid)
{
    std::println ("{}", *tid);
}

int main()
{
    std::vector<scl::thread> threads { };

    {
        // Launch all threads.
        const int num_threads = 15;

        for (int i : std::views::iota (0, num_threads))
        {
            auto i_ptr = &i;
            threads.push_back (scl::thread (entry_point, i_ptr));
        }
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}