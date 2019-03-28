
# zf_check

A minimalist error handling approach that enables unobtrusive yet rigorous error checking in C.
Inspired by and builds upon [zf_log](https://github.com/wonder-mice/zf_log). Though designed to work
well with zf_log, you do not need to use it to use zf_check.

# Purpose

Correct coding in C is very difficult. Error handling in C is painfully manual. This module is
designed to encourage good coding practices by lowering the barrier to perform thorough error
handling.

# Example

(TODO create better examples)
```c
  int rv = foo();
  ZF_CHECK(0 > rv, ZF_CRIT, ZF_CLEANUP, false, "foo returned a bad value! (%d)", rv);
```
instead of
```c
  int rv = foo();
  if (0 > rv) {
    ZF_LOG(ZF_CRITICAL, "foo returned a bad value! (%d)", rv);
    success = false;
    goto cleanup;
  }
```

# Wishlist
- ZF_CHECK variant that doesn't require `status`
- Support for multiple logging options
  - zf_log
- Improved README
  - Usage
  - Purpose, importance of defensive coding
- Improved Makefile, or CMake
- Run-time and build-time configuration
- Module generator that uses template files to create modules preconfigured for zf_check
