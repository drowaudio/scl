//
// Created by David Rowland on 24/01/2025.
//

#pragma once

#include <atomic>

// No Data race:
// A reader enters a function and there is an active reader
//
// Data race:
// A reader enters a function and there is an active writer
// A writer enters a function and there is an active writer
// A writer enters a function and there is an active reader
//
// Wait-free? (maybe the CMPX is only lock-free?)

//==========================================
//==========================================
struct check_state
{
    std::atomic<size_t> num_readers { 0 };
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
    ++state.num_readers; // must be first

    if (state.is_writing)
        std::terminate();
        // read during active write
}

void write_started (check_state& state)
{
    if (state.is_writing.exchange (true))  // must be first
        std::terminate();
        // write during active write

    if (state.num_readers > 0)
        std::terminate();
        // write during active read
}

void read_ended (check_state& state)
{
    --state.num_readers;
}

void write_ended (check_state& state)
{
    state.is_writing = false;
}


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
#include "extrinsic_storage.h"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <source_location>
#include <thread>
#include <vector>

template<typename Tag = check_state>
class date_race_registry {
    static inline auto tags    = extrinsic_storage<Tag>{};
    static inline auto log     = std::ofstream{ "data-race-violations.log" };
public:

    static inline auto get_state(void* pobj) noexcept {
        return *tags.find_or_insert(pobj);
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

//    static inline auto on_get_alternative(void* pobj, uint32_t alt, std::source_location where = std::source_location::current()) -> void {
//        if (auto active = tags.find(pobj);
//            active                  // if we have discriminator info for this union
//            && *active != alt       // and the discriminator not what is expected
//            && *active != unknown   // and is not unknown
//            )
//        {
//            log << where.file_name() << '(' << where.line()
//                << "): union type safety violation - active member " << (*active == invalid ? "invalid" : std::to_string(*active))
//                << ", attempted to access " << alt << "\n";
//        }
//    }
};
