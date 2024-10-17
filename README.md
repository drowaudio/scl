# scl - Safe Concurrency Library*

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

