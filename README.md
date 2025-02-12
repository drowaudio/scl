# scl - Safe Concurrency Library*
[![test](https://github.com/drowaudio/scl/actions/workflows/test.yaml/badge.svg)](https://github.com/drowaudio/scl/actions/workflows/test.yaml)

This is an experiment to see how much of sync and send from Rust/Swift can be implemented in C++ and what problems arise when we try to do so. Is there anything we can learn about thread safety from these approaches.

- The main sync/send concepts are in [sync_send.h](sync_send.h)
- They are used in [safe_thread.h](safe_thread.h)
- They are tested in [tests](tests)

###### *Safe is the goal of the project, not the current status 

### 💡Key Ideas:
- It's possible to implement a very restrictive sync/send type trait
- It's more restrictive that _could be_ due to C++ aliasing rules. I.e. there's no way to ensure a const& doesn't have non-const references elsewhere. This is where borrow checking really outshines C++
- It's only possible to get surface level (i.e. one level deep) by directly checking the arguments. Other languages check this recursively. It is hoped this might become more feasible with reflection in C++
- Template errors can be difficult to navigate
- Just having to think about the types passed to scl types (e.g. safe_thread) can help the user think in a more thread-safe way

___
### Supported Platforms
- Clang 18
- GCC 14.2
- AppleClang 16

___
## To Do:
### `sync`/`send`
- [x] `scl::thread` - A safe thread that encapsulates running a thread and checks arguments to that thread conform to the send trait
- [x] `scl::async` - Similar to `scl::thread` but around `std::async` 
- [x] `scl::synchronized_value` - A wrapper around a mutex and an object to provide safe concurrent access to it, conforms to the `sync` trait
- [ ] Reflection based implementation that checks `sync` recursively
### Data race checker
- [ ] `check_state` and `scoped_check` to manually check for data-races on function calls
- [ ] `date_race_registry` to avoid having to use a `check_state` member
### Meta-classes
- [ ] `data_race_checked` - Checks for data races during every function call
- [ ] `mutex` - Locks access during every function call, conforms to `sync`
- [ ] `shared_mutex` - Locks shared access during every const function call, unique access otherwise, conforms to `sync`
- [ ] `cow` copy-on-write
- [ ] `arc` automatic-reference-counting
- [ ] `actor` actor implementation using senders
