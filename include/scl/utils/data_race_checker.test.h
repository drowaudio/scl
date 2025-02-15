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

  // Test delegating a write-write
  void push_back_2(const T& v1, const T& v2) {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    push_back(v1);
    push_back(v2);
  }

  // Test delegating a write-read
  T& push_back_and_return(const T& value) {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.push_back(value);
    return data.back();
  }

  void pop_back() {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));
    data.pop_back();
  }

  // Test delegating a write-read
  const T& push_back_return_old_back(const T& value) {
    scl::scoped_check<scl::check_type::write> _ (scl::data_race_registry<>::get_state (this));

    auto& old_back = back();
    push_back(value);
    return old_back;
  }

  T& back() {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data.back();
  }

  const T& back() const {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return data.back();
  }

  // Test delegating a read-write
  // This wouldn't normally occur as you'd have a const function calling a non-const function
  const T& back_with_default_write() {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    push_back(T());
    return data.back();
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

  // Test delegating a read-read
  std::pair<size_t, size_t> size_and_capacity() const
  {
    scl::scoped_check<scl::check_type::read> _ (scl::data_race_registry<>::get_state (this));
    return { size(), capacity() };
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

inline void test_no_data_race_read_read()
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

                           auto sz_cp = vec.size_and_capacity();
                           c = sz_cp.first;
                           c = sz_cp.second;
                        }
                      });

  for (auto& t : threads)
    t.join();
}

inline void test_no_data_race_read_write()
{
  test_vector<size_t> vec;
  volatile auto c = 0uz;
  vec.push_back (auto (c));
  c = vec.back_with_default_write();
}

inline void test_no_data_race_write_read()
{
  test_vector<size_t> vec;
  volatile auto c = 0uz;
  vec.push_back(auto(c));
  c = vec.push_back_return_old_back(auto (c));
}

inline void test_no_data_race_write_write()
{
  test_vector<size_t> vec;
  volatile auto c = 0uz;
  vec.push_back_2(auto (c), auto (c));
}
