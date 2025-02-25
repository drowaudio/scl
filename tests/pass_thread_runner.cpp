#include <functional>
#include <memory>
#include <print>
#include <scl/safe_thread.h>

/**
    A wrapper that can be used around pointers to allow the this pointer to be passed.
    There's nothing special about this, it's just a dump pointer but means we can
    explicitly allow it when we get reflection and makes it obvious in calling
    code that the intent is to share it.
 */
template<typename T>
class unchecked_pointer
{
public:
    unchecked_pointer (nullptr_t) = delete;

    unchecked_pointer (T* p)
        : pointer (p) {
    }

    unchecked_pointer (T& p)
        : pointer (*p) {
    }

    T& operator*() const {
        return *pointer;
    }

    T* operator->() const {
        return pointer;
    }

private:
    T* pointer;
};

template<typename T>
struct scl::is_send<unchecked_pointer<T>> : std::true_type {};

using namespace std::literals;

class thread_runner
{
public:
    thread_runner()
    {
        std::mem_fn (&thread_runner::run) (unchecked_pointer<thread_runner> (this));

        // static_assert(is_send_v<decltype(std::mem_fn (&thread_runner::run))>);
        // static_assert(is_send_v<std::decay_t<decltype(std::mem_fn (&thread_runner::run))>>);
        thread = std::make_unique<scl::thread> (std::mem_fn (&thread_runner::run), unchecked_pointer (this));
        // thread = std::make_unique<safe_thread> (std::mem_fn (&thread_runner::run), this_wrapper<const thread_runner> (const_cast<const thread_runner*> (this)));
        // thread = std::make_unique<safe_thread> (std::mem_fn (&thread_runner::run), const_cast<const thread_runner*> (this));
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
    thread_runner tr;
}