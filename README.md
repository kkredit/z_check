# z_check
z_check is an error handling library to enable unobtrusive yet rigorous C error checking. Error
checking in C is verbose and annoying, but it is also crucial for writing safe and secure programs.
This library tries to help.

Primary goal: lower the cognitive load for performing strong error checking.

Secondary goals: simple but capable, small source size, standardized way of doing things.

>Perfection is achieved not when there is nothing more to add, but when there is nothing left to
>take away.  
>\- Antoine de Saint-Exupery


## Features
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
- Debug variants of each macro that compile-out when `NDEBUG` is defined
- Compiles with `-std=c99` and strict warnings enabled


## Example
See [example.c](examples/example.c), which exercises most of the library's
functionality.

As a basic example, this code:
```c
  int rv = foo();
  if (0 > rv) {
    Z_LOG(Z_ERR, "foo returned a bad value! (%d)", rv); # even this is non-trivial
    status = -1;
    goto cleanup;
  }
```
becomes:
```c
  int rv = foo();
  Z_CHECK(0 < rv, -1, Z_ERR, "foo returned a bad value! (%d)", rv);
```


## Wishlist
- `Z_CHECK` variant that doesn't require `status` variable
- Logging support for zf_log (wip)
- Improve README
  - Background: importance of defensive coding, good practices
  - How to integrate into your own project
- Example of some real file, before and after
- Improve Makefile, move to CMake
  - Separate z_check from examples; probably static lib
  - CMake for cleanliness and portability
- More examples
  - Programs that exemplify exactly one feature, to contrast with one example that has all
- Refactor/cleanup/consolidate after adding new functionality
  - And move some functionality out to branches
  - Z_CHECKG to own branch; not preferred because obscures logic
  - Z_CHECKC to own branch; not preferred because very rarely needed
  - Z_ZFLOG to own branch; not preferred because so rarely going to be used


## Name
The "z" in z_check is inspired by [zf_log](https://github.com/wonder-mice/zf_log), a nice logging
library that z_check supports as one of its logging options.
