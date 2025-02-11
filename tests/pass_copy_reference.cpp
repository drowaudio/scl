#include <functional>
#include <memory>
#include <print>
#include <ranges>
#include <scl/safe_thread.h>

using namespace std::literals;

void entry_point (int tid)
{
    std::println ("{}", tid);
}

static_assert(std::is_function_v<decltype(entry_point)>);
static_assert(! std::is_object_v<decltype(entry_point)>);
static_assert(! std::is_member_function_pointer_v<decltype(entry_point)>);
static_assert(! std::is_pointer_v<decltype(entry_point)>);

static_assert(scl::is_function_pointer_v<std::decay_t<decltype(entry_point)>>
                && ! std::is_member_function_pointer_v<decltype(entry_point)>);
static_assert(scl::is_function_pointer_v<std::decay_t<decltype(entry_point)>>
                && ! std::is_member_function_pointer_v<decltype(entry_point)>);

int main()
{
    std::vector<scl::thread> threads { };

    {
        // Launch all threads.
        const int num_threads = 15;

        for (int i : std::views::iota (0, num_threads))
            threads.push_back (scl::thread (entry_point, auto (i)));
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}