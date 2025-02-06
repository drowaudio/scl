#include <functional>
#include <memory>
#include <print>
#include "../safe_thread.h"

using namespace std::literals;



class thread_runner
{
public:
    thread_runner()
    {
        std::mem_fn (&thread_runner::run) (unchecked_pointer<thread_runner> (this));

        // static_assert(is_send_v<decltype(std::mem_fn (&thread_runner::run))>);
        // static_assert(is_send_v<std::decay_t<decltype(std::mem_fn (&thread_runner::run))>>);
        thread = std::make_unique<safe_thread> (std::mem_fn (&thread_runner::run), unchecked_pointer (this));
        // thread = std::make_unique<safe_thread> (std::mem_fn (&thread_runner::run), this_wrapper<const thread_runner> (const_cast<const thread_runner*> (this)));
        // thread = std::make_unique<safe_thread> (std::mem_fn (&thread_runner::run), const_cast<const thread_runner*> (this));
    }

private:
    std::unique_ptr<safe_thread> thread;

    void run() const
    {
        std::println("PROCESSING...");
        std::this_thread::sleep_for (1s);
        std::println("...DONE");
    }
};

int main() {
    thread_runner tr;
}