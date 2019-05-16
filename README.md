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
- `Z_CHECK`: one-liner error check, logging command, and goto
- `Z_LOG` and variants: one-liner logging command, configurably printing to
    - stdout
    - stderr
    - syslog
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
  Z_CHECK(0 > rv, -1, Z_ERR, "foo returned a bad value! (%d)", rv);
```


## Integration
The simplest way to integrate z_check into your project is to copy the two source files to your
project and integrate with your build system. For small programs, build and link directly. For
larger programs, z_check makes a good static library.

## Branches
There exist a few branches with additional features. These features are excluded from master because
either they would not be used in most projects or they violate the design value of simplicity.
- `variants`: contains a few different flavors of Z_CHECK:
    - `Z_CHECKG`: you name the label to 'goto'
    - `Z_CHECKC`: even if the condition evaluates to true, do not 'goto'
- `xentoollog`: contains support for xentoollog as a logging target (wip)
- `zf_log`: contains support for [zf_log](https://github.com/wonder-mice/zf_log) as a logging target
    (wip)


## Wishlist
- Logging support for zf_log (wip; to remain in own branch)
- Improve README
  - Background: importance of defensive coding, good practices
- Example of some real file, before and after
- Improve Makefile, move to CMake
  - Separate z_check from examples; probably static lib
  - CMake for cleanliness and portability
- More examples
- Refactor/cleanup/consolidate after adding new functionality
  - And move some functionality out to branches
- Anything else? PRs welcome!


## Name
The "z" in z_check is inspired by [zf_log](https://github.com/wonder-mice/zf_log), a nice logging
library that z_check supports as one of its logging options (see zf_log branch).
