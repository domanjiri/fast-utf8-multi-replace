# High throughput and fast UTF-8 multiple replace

A task-based parallel search and replace which also benefits from concurrency at the register level__i.e, SIDM.

## Requirements

* GNU compiler with support for C++17 or C++2a
* CMake 3.10 and later
* An SSE4.2 support processor
* Intel`s Threading Building Blocks library

## Building

```shell
$ git clone https://github.com/domanjiri/fast-utf8-multi-replace.git
$ mkdir build
$ cd build
$ cmake ../
$ make
```

## Running Tests

When the project builds successful, Tests could be run from the build directory with this command:

```shell
$ ./test/replace_test
```


## Sample data
Any valid Unicode text file can be used for search and replace. For dictionary file, look the test directory.


## Credits

The SIMD part of the code is in indebtedness to the work of Olivier Goffart [here](https://woboq.com/blog/utf-8-processing-using-simd.html).

