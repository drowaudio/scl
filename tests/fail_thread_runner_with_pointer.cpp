#include <functional>
#include <memory>
#include <print>
#include <scl/safe_thread.h>

using namespace std::literals;

class thread_runner
{
public:
    thread_runner()
    {
        static_assert(! scl::is_send_v<decltype([this] { return this; })>);
        int i = 42;
        auto i_ptr = &i;
        thread = std::make_unique<scl::thread> (std::mem_fn (&thread_runner::run), this, i_ptr);
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