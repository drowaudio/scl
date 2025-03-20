//
// Created by David Rowland on 24/01/2025.
//

#pragma once

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <source_location>
#include <thread>
#include <vector>
#include <atomic>

#ifdef __GNUC__
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#include "extrinsic_storage.h"

#ifdef __GNUC__
 #pragma GCC diagnostic pop
#endif

// No Data race:
// A reader enters a function and there is an active reader
//
// Data race:
// A reader enters a function and there is an active writer
// A writer enters a function and there is an active writer
// A writer enters a function and there is an active reader
//
// N.B. This checks for recursion but there is a false positive if:
// - A write starts
// - A read on a separate thread starts
// - That read finishes
// - The original write continues but a data race is flagged
// Although there is technically no race, this is probably unintended behaviour
//
// Wait-free? (maybe the CMPX is only lock-free?)

/** @note
 *  There's a bit of a debate if using the stack pointer (address of a local variable)
 *  is quicker than getting the thread ID. A rudimentary benchmark (below) shows
 *  getting this is the same on clang 17 and GCC 13.2:
 *  https://quick-bench.com/q/iTnk1X3mGYUQ2yryuK_Wcg6JJ6Q
 *
 *  When detecting if there is a data-race, comparing thread IDs is very quick. I
 *  would expect finding if two addresses are on the same stack is more costly as
 *  it involves extra calls to get the base stack address. It's also more complex
 *  in terms of code. Ultimately however, real-word benchmarks would need to be
 *  created to compare without a data_race_checker, one using thread IDs and one
 *  using addresses.
*/

namespace scl {

#ifndef DATA_RACE_DETECTED
    #define DATA_RACE_DETECTED std::abort();
#endif

//==========================================
//==========================================
struct check_state
{
    std::atomic<size_t> num_readers { 0 };
    std::atomic<std::thread::id> last_thread_id { std::thread::id() };
    std::atomic<bool> is_writing { false };
};

bool can_write (const check_state& state)
{
    return state.num_readers == 0;
}

bool can_read (const check_state& state)
{
    return ! state.is_writing;
}

//==========================================
void read_started (check_state& state)
{
    auto this_thread_id = std::this_thread::get_id();
    auto last_thread_id = state.last_thread_id.exchange (this_thread_id);

    ++state.num_readers; // must be first

    if (state.is_writing)
    {
        if (last_thread_id != this_thread_id)
        { DATA_RACE_DETECTED }
        // read during active write
    }
}

void write_started (check_state& state)
{
    auto this_thread_id = std::this_thread::get_id();
    auto last_thread_id = state.last_thread_id.exchange (this_thread_id);

    if (state.is_writing.exchange (true))  // must be first
    {
        if (last_thread_id != this_thread_id)
        { DATA_RACE_DETECTED }
        // write during active write
    }

    if (state.num_readers > 0)
    {
        if (last_thread_id != this_thread_id)
        { DATA_RACE_DETECTED }
        // write during active read
    }
}

void read_ended (check_state& state)
{
    --state.num_readers;
}

void write_ended (check_state& state)
{
    state.is_writing = false;
}

#undef DATA_RACE_DETECTED

//==========================================
//==========================================
enum class check_type
{
    read,
    write
};

//==========================================
template<check_type type>
struct scoped_check
{
    scoped_check (check_state& check_state)
        : state (check_state)
    {
        if constexpr (type == check_type::read)
            read_started (state);
        else
            write_started (state);
    }

    ~scoped_check()
    {
        if constexpr (type == check_type::read)
            read_ended (state);
        else
            write_ended (state);
    }

    check_state& state;
};


//==========================================
//==========================================
template<typename Tag = check_state>
class data_race_registry {
    static inline auto tags    = extrinsic_storage<Tag>{};
    static inline auto log     = std::ofstream{ "data-race-violations.log" };
public:

    static inline auto& get_state(const void* pobj) noexcept {
        // This const cast should be tidied up
        return *tags.find_or_insert(const_cast<void*> (pobj));
    }

    static inline auto on_destroy(void* pobj) noexcept -> void { tags.erase(pobj); }


    static inline auto on_read_started(void* pobj) noexcept -> void {
        if (auto p = tags.find_or_insert(pobj)) { read_started (*p); }
    }

    static inline auto on_write_started(void* pobj) noexcept -> void {
        if (auto p = tags.find_or_insert(pobj)) { write_started (*p); }
    }

    static inline auto on_read_ended(void* pobj) noexcept -> void {
        if (auto p = tags.find_or_insert(pobj)) { read_ended (*p); }
    }

    static inline auto on_write_ended(void* pobj) noexcept -> void {
        if (auto p = tags.find_or_insert(pobj)) { write_ended (*p); }
    }
};

}
