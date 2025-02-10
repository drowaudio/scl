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
        thread = std::make_unique<scl::thread> ([this] { run(); });
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
}