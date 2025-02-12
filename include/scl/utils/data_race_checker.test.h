//
// Created by David Rowland on 11/02/2025.
//

#include "data_race_checker.h"
#include <vector>
#include <thread>
#include <span>
#include <atomic>

template <typename T>
class test_vector
{
public:
  test_vector() {
    // Create the entry (might allocate)
    scl::data_race_registry<>::get_state (this);
  }

  ~test_vector() {
    scl::data_race_registry<>::on_destroy (this);
  }

  void reserve (size_t new_cap) {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.resize (new_cap);
  }

  void clear() {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.clear();
  }

  void push_back(const T& value) {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.push_back(value);
  }

  void pop_back() {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.pop_back();
  }

  T& operator[](size_t index) {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data[index];
  }

  size_t size() const {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data.size();
  }

  bool empty() const {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data.empty();
  }

  size_t capacity() const {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data.capacity();
  }

private:
  std::vector<T> data;
};

inline void test_no_data_race()
{
  test_vector<size_t> vec;

  // Fill the vector first
   for (auto c : std::ranges::iota_view (0uz, 1'000uz))
     vec.push_back (c);

  // Then only read from the vector - no data race
std::vector<std::thread> threads;

  for (auto _ : std::ranges::iota_view (0, 3))
    threads.emplace_back([&]
                       {
                         for (auto _ : std::ranges::iota_view (0uz, 1'000'000uz))
                         {
                           volatile auto c = 0uz;

                           if (! vec.empty())
                             c = vec[vec.size() - 1];
                        }
                      });

  for (auto& t : threads)
    t.join();
}

inline void test_data_race()
{
  std::vector<std::thread> threads;
  test_vector<size_t> vec;

  threads.emplace_back([&]
                       {
                         for (;;)
                         {
                           for (auto c : std::ranges::iota_view (0uz, 1'000uz))
                             vec.push_back (c);

                         vec.clear();
                       }
                     });

  for (auto _ : std::ranges::iota_view (0, 3))
    threads.emplace_back([&]
                       {
                         for (;;)
                         {
                           volatile auto c = 0uz;

                           if (! vec.empty())
                             c = vec[vec.size() - 1];
                        }
                      });

  for (auto& t : threads)
    t.join();
}
