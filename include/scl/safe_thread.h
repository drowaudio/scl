//
// Created by David Rowland on 17/09/2024.
//

#pragma once

#include "sync_send.h"
#include <thread>
#include <utility>

namespace scl {
/**
 *  A thread whose invokable and argumnets are checked to ensure they conform
 *  to the send concept.
 *  This introduces an isolation boundry to try and force only safe objects
 *  to pass.
 *
 *  Notably in C++ we can't go any deeper than surface level so this only
 *  provides minimal protection. It is hoped with reflection we can recursively
 *  check arguments to ensure greater safety.
 */
class thread
{
public:
    template<typename F, send... Args>
    thread (F&& f, Args&&... args)
        : thread_internal (std::forward<F> (f), std::forward<Args> (args)...)
    {
        // N.B. We can't constrain F to the concept due to recursion of is_move_constructable
        // So we have to statically assert it
        static_assert (send<F>);
    }

    thread (thread&& other)
        : thread (std::move (other.thread_internal))
    {
    }

    ~thread()
    {
        if (thread_internal.joinable())
            join();
    }

    void join()
    {
        thread_internal.join();
    }

private:
    std::thread thread_internal;
};
}
