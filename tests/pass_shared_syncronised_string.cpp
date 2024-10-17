#include <functional>
#include <memory>
#include <print>
#include <ranges>
#include "../safe_thread.h"
#include "../syncronized_value.h"

using namespace std::literals;

void entry_point (std::shared_ptr<synchronized_value<std::string>> sync_s, int tid)
{
    apply ( [tid] (auto& s) {
        s.append ("🔥");
        std::println ("{} {}", s, tid);
        return s;
    },
    *sync_s);
}

static_assert(is_function_pointer_v<std::decay_t<decltype(entry_point)>>);
static_assert(! std::is_member_function_pointer_v<std::decay_t<decltype(entry_point)>>);
static_assert(! std::is_member_function_pointer_v<decltype(entry_point)>);
static_assert(is_send_v<decltype(entry_point)>);

int main()
{
    std::vector<safe_thread> threads { };

    {
        //s dies before the threads join, so possible
        auto s = std::make_shared<synchronized_value<std::string>> ("Hello threads");

        // Launch all threads.
        const int num_threads = 15;

        for (int i : std::views::iota (0, num_threads))
            threads.push_back (safe_thread (entry_point, auto (s), auto (i)));
    }

    // Join all threads.
    for (safe_thread& t : threads)
        t.join();
}