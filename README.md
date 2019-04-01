# z_check
An error handling library to enable unobtrusive yet rigorous C error checking. Error checking in C
is verbose and annoying, but it is also crucial for writing safe and secure programs. This library
tries to lessen the pain.

The goal of this library is to be as small and generally applicable as possible.

>Perfection is achieved not when there is nothing more to add, but when there is nothing left to
take away.

\- Antoine de Saint-Exupery


# Features
- Run-time and build-time library configuration
- Run-time modification of logging levels (helps with noise)
- `Z_CT_ASSERT`: simple compile-time asserts
- `Z_RT_ASSERT`: simple run-time asserts (friendly wrapper around `assert()`)
- `Z_CHECK` and variants: one-liner error check, logging command, and goto
- `Z_LOG` and variants: one-liner logging command, configurably printing to
    - stdout
    - stderr
    - syslog
    - zf_log (wip)
- Compiles with `-std=c99` and strict warnings enabled


# Example
See [example.c](examples/example.c), which exercises most of the library's
functionality.

As a basic example, this code:
```c
  int rv = foo();
  Z_CHECK(0 < rv, -1, Z_ERR, "foo returned a bad value! (%d)", rv);
```
is equivalent to:
```c
  int rv = foo();
  if (0 > rv) {
    Z_LOG(Z_LOG_ERROR, "foo returned a bad value! (%d)", rv);
    status = -1;
    goto cleanup;
  }
```
where `Z_LOG()` itself encapsulates non-trivial logging functionality.


# Wishlist
- `Z_CHECK` variant that doesn't require `status` variable
- Logging support for zf_log (wip)
- Background section in README: purpose, importance of defensive coding, good practices
- Example of some real file, before and after
- Improve Makefile, move to CMake


# Name
The "z" in z_check is inspired by [zf_log](https://github.com/wonder-mice/zf_log), a nice logging
library that z_check supports as one of its logging options.
