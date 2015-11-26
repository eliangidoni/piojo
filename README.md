[![Build Status](https://travis-ci.org/eliangidoni/piojo.svg?branch=master)](https://travis-ci.org/eliangidoni/piojo)

[![Coverage Status](https://coveralls.io/repos/eliangidoni/piojo/badge.svg?branch=master&service=github)](https://coveralls.io/github/eliangidoni/piojo?branch=master)

```
    Piojo is a small C (C99) library of common containers and other useful
    structures.

    Documentation: http://eliangidoni.github.io/piojo


                                  Dependencies

1) CMake 2.6 (or any later version).
2) (optional) Doxygen for docs generation.
3) (optional) gcov/lcov for code coverage.


                                     Build

$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Release ..
$ make

To generate docs:
$ make doc


                                     Usage

See directory doc/ or test/ for code samples.


                                  Development

1) Debug

$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Debug ..
$ make

2) Testing / Coverage

$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local -DCMAKE_BUILD_TYPE=Testing ..
$ make
$ CTEST_OUTPUT_ON_FAILURE=1 make test

3) Code Style

- Avoid lines with more than 80 characters.
- Avoid more than 3 levels of indentation.
- Avoid functions longer than 50 lines.
- Avoid functions with more than 5 arguments.
- Predicate (boolean) functions end with '_p'.
- Interface type names end with '_if'.
- Function/Callback type names end with '_cb'.
- Any other type names end with '_t'.
- Don't use typedef to hide pointer types. (exception: functions/callbacks)

```

