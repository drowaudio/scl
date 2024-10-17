#include <functional>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <thread>
#include <vector>
#include "safe_thread.h"
#include "syncronized_value.h"

using namespace std::literals;


// Custom class that is "sync" because it uses a mutex
// class SynchronizedCounter {
// public:
//     void increment() {
//         std::lock_guard<std::mutex> guard(mutex_);
//         ++counter_;
//     }
//
//     int get() const {
//         std::lock_guard<std::mutex> guard(mutex_);
//         return counter_;
//     }
//
// private:
//     mutable std::mutex mutex_;  // Protects the counter for thread-safe access
//     int counter_ = 0;
// };
//
// // Custom class that is "send" because it is trivially movable
// class MovableData {
// public:
//     MovableData(int value) : data_(value) {}
//
//     MovableData(MovableData&& other) noexcept : data_(other.data_) {
//         other.data_ = 0;
//     }
//
//     MovableData& operator=(MovableData&& other) noexcept {
//         if (this != &other) {
//             data_ = other.data_;
//             other.data_ = 0;
//         }
//         return *this;
//     }
//
//     int get() const { return data_; }
//
// private:
//     int data_;
// };
//
// // Helper function to test sync and send traits
// template <typename T>
// void check_traits() {
//     if (is_sync<T>::value) {
//         std::cout << "Type is sync.\n";
//     } else {
//         std::cout << "Type is not sync.\n";
//     }
//
//     if (is_send<T>::value) {
//         std::cout << "Type is sendable.\n";
//     } else {
//         std::cout << "Type is not sendable.\n";
//     }
// }
//
// int main() {
//     std::cout << "Checking SynchronizedCounter:\n";
//     check_traits<SynchronizedCounter>();
//
//     std::cout << "\nChecking MovableData:\n";
//     check_traits<MovableData>();
//
//     std::cout << "\nChecking int:\n";
//     check_traits<int>();
//
//     return 0;
// }


// void thread_function(int thread_id) {
//     std::cout << "Thread " << thread_id << " is running\n";
// }
//
// int main() {
//     // Number of threads to create
//     const int num_threads = 5;
//
//     // Create a vector to store threads
//     std::vector<std::thread> threads;
//
//     // Start threads and push them into the vector
//     for (int i = 0; i < num_threads; ++i) {
//         threads.push_back(std::thread(thread_function, i));  // Push a new thread into the vector
//     }
//
//     // Join all threads to ensure they complete execution before main continues
//     for (auto& thread : threads) {
//         if (thread.joinable()) {
//             thread.join();  // Join the thread to the main thread
//         }
//     }
//
//     std::cout << "All threads have finished execution\n";
//
//     return 0;
// }


// Can we pass mutable borrows into thread entry
// void entry_point (const std::string& s, int tid)
// {
//     // s.append ("More text");
//     std::println ("{} {}", s, tid);
// }

// void entry_point (std::shared_ptr<synchronized_value<std::string>> sync_s, int tid)
// {
//     apply ( [tid] (auto& s) {
//         s.append ("🔥");
//         std::println ("{} {}", s, tid);
//         return s;
//     },
//     *sync_s);
// }

// void entry_point (std::shared_ptr<std::string> s, int tid)
// {
//     s->append ("More text");
//     std::println ("{} {}", *s, tid);
// }
// // standard c++
// int main()
// {
//     std::vector<std::thread> threads { };
//
//     {
//         //s dies before the threads join, so use-after free
//         std::string s = "Hello threads";
//
//         // Launch all threads.
//         const int num_threads = 15;
//
//         for (int i : std::views::iota (0, num_threads))
//             threads.push_back (std::thread (entry_point, s, i));
//     }
//
//     // Join all threads.
//     for (std::thread& t : threads)
//         t.join();
// }

// standard c++ lambda
// int main()
// {
//     std::vector<std::thread> threads { };
//
//     {
//         //s dies before the threads join, so use-after free
//         std::string s = "Hello threads";
//
//         // Launch all threads.
//         const int num_threads = 15;
//
//         for (int i : std::views::iota (0, num_threads))
//             threads.emplace_back ([&s, i] { entry_point (s, i); });
//     }
//
//     // Join all threads.
//     for (std::thread& t : threads)
//         t.join();
// }

// int main()
// {
//     std::vector<safe_thread> threads { };
//
//     {
//         //s dies before the threads join, so possible
//         auto s = std::make_shared<synchronized_value<std::string>> ("Hello threads");
//
//         // Launch all threads.
//         const int num_threads = 15;
//
//         for (int i : std::views::iota (0, num_threads))
//             threads.push_back (safe_thread (entry_point, auto (s), auto (i)));
//     }
//
//     // Join all threads.
//     for (safe_thread& t : threads)
//         t.join();
// }

//==================================================================
// void entry_point (std::shared_ptr<std::string> s, int tid)
// {
//     s->append ("🔥");
//     std::println ("{} {}", *s, tid);
// }
//
// int main()
// {
//     std::vector<safe_thread> threads { };
//
//     {
//         //s dies before the threads join, so possible
//         auto s = std::make_shared<std::string> ("Hello threads");
//
//         // Launch all threads.
//         const int num_threads = 15;
//
//         for (int i : std::views::iota (0, num_threads))
//             threads.push_back (safe_thread (entry_point, auto (s), auto (i)));
//     }
//
//     // Join all threads.
//     for (safe_thread& t : threads)
//         t.join();
// }

// #feature on safety
// #include <https://raw.githubusercontent.com/cppalliance/safe-cpp/master/libsafecxx/single-header/std2.h>
//
// using namespace std2;
//
// // Can we pass mutable borrows into thread entry
// void entry_point(string^s, int tid) safe {
//     s^->append ("More text");
//     // println (*s) ;
// }
//
// int main () safe
// {
//     vector<thread> threads { };
//
//         {
//         //s dies before the threads join, so possib
//         string s = "Hello threads";
//
//         // Launch all threads.
//         const int num_threads = 15;
//         for(int i : num threads)
//             threads^. push_back (thread (&entry_point, ^s));
//         }
//
//     // Join all threads.
//     for(thread^t : threads)
//         t^->join();
// }


//==================================================================
// class ThreadRunner
// {
// public:
//     ThreadRunner()
//     {
//         thread = std::make_unique<safe_thread> (std::mem_fn (&ThreadRunner::run), const_cast<const ThreadRunner*> (this));
//     }
//
// private:
//     std::unique_ptr<safe_thread> thread;
//
//     void run() const
//     {
//         std::println("PROCESSING...");
//         std::this_thread::sleep_for (1s);
//         std::println("...DONE");
//     }
// };
//
// int main() {
//     ThreadRunner tr;
// }

int main() {

}