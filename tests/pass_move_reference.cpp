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

using T = decltype(entry_point);
static_assert (std::is_pointer_v<std::remove_extent_t<std::decay_t<T>>>);
static_assert (scl::is_function_pointer_v<std::decay_t<T>>);
static_assert (! (std::is_pointer_v<std::remove_extent_t<std::decay_t<T>>>
                  && ! scl::is_function_pointer_v<std::decay_t<T>>));
static_assert (! std::is_member_function_pointer_v<T>);

static_assert (! std::is_move_constructible_v<T>);
static_assert (scl::is_function_pointer_v<std::decay_t<T>>
               && ! std::is_member_function_pointer_v<std::decay_t<T>>);
static_assert (! scl::is_sync_v<T>);

static_assert(std::is_function_v<void (int)>);
static_assert(std::is_function_v<std::remove_cvref_t<void (&)(int)>>);

//======================================================
int main()
{
    std::vector<scl::thread> threads { };

    {
        // Launch all threads.
        const int num_threads = 15;

        for (int i : std::views::iota (0, num_threads))
            threads.push_back (scl::thread (entry_point, std::move (i)));
    }

    // Join all threads.
    for (scl::thread& t : threads)
        t.join();
}