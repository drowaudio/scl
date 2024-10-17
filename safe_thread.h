//
// Created by David Rowland on 17/09/2024.
//

#pragma once

#include "sync_send.h"
#include <thread>
#include <utility>

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
class safe_thread
{
public:
    template<typename F, send... Args>
    safe_thread (F&& f, Args&&... args)
        : thread (std::forward<F> (f), std::forward<Args> (args)...)
    {
        // N.B. We can't constrain F to the concept due to recursion
        // We also have to explcitly allow non-member function pointers as
        // these aren't found via the send trait for some reason
        static_assert((is_function_pointer_v<std::decay_t<std::decay_t<F>>>
                       && ! std::is_member_function_pointer_v<std::decay_t<F>>)
                      || is_send_v<std::decay_t<F>>);

        // This is what we really want to say, in a concept
        // static_assert (is_send_v<std::decay_t<F>>);
    }

    safe_thread (safe_thread&& other)
        : thread (std::move (other.thread))
    {
    }

    ~safe_thread()
    {
        if (thread.joinable())
            join();
    }

    void join()
    {
        thread.join();
    }

private:
    std::thread thread;
};