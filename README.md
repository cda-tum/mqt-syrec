[![CI](https://github.com/iic-jku/syrec/actions/workflows/ci.yml/badge.svg)](https://github.com/iic-jku/syrec/actions/workflows/ci.yml)
[![Codecov branch](https://img.shields.io/codecov/c/github/iic-jku/syrec/master?label=codecov&logo=codecov&style=plastic&token=JQO5NqY4oM)](https://codecov.io/gh/iic-jku/syrec)

# SyReC - A Programming Language for the Synthesis of Reversible Circuits

The tool is based on methods proposed in [[1]](https://iic.jku.at/files/eda/2019_iccad_hdl_based_reversible_circuit_synthesis_without_additional_lines.pdf).

[[1]](https://iic.jku.at/files/eda/2019_iccad_hdl_based_reversible_circuit_synthesis_without_additional_lines.pdf) Robert Wille, Majid Haghparast, Smaran Adarsh, and Tanmay M. **"Towards HDL-based Synthesis of Reversible Circuits with No
Additional Lines"**. In International Conference on Computer Aided Design (ICCAD), 2019

TODO: Description of what the tool is capable of and references to further relevant publications.

## Usage

SyReC is mainly developed as a C++ library with an easy-to-use Python interface.

- In order to make the library as easy to use as possible (without compilation), we provide pre-built wheels for most common platforms (64-bit Linux, MacOS, Windows). These can be installed using
    ```bash
    pip install syrec
    ```
  However, in order to get the best performance out of SyReC, it is recommended to build it locally from the source distribution (see [system requirements](#system-requirements)) via
    ```bash
    pip install syrec --no-binary syrec
    ```
  This enables platform specific compiler optimizations that cannot be enabled on portable wheels.
- Once installed, start using it in Python:
    ```python
    from tumqit.syrec import * 
    
    <...>
    ```

### System requirements

Building (and running) is continuously tested under Linux, MacOS, and Windows using the [latest available system versions for GitHub Actions](https://github.com/actions/virtual-environments). However, the implementation should be compatible
with any current C++ compiler supporting C++17 and a minimum CMake version of 3.14.

*Disclaimer*: We noticed some issues when compiling with Microsoft's `MSCV` compiler toolchain. If you are developing under Windows, consider using the `clang` compiler toolchain. A detailed description of how to set this up can be
found [here](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-160).

### Setup, Configure, and Build

To start off, clone this repository using

```shell
git clone --recurse-submodules -j8 https://github.com/iic-jku/syrec 
```

Note the `--recurse-submodules` flag. It is required to also clone all the required submodules. If you happen to forget passing the flag on your initial clone, you can initialize all the submodules by
executing `git submodule update --init --recursive` in the main project directory.

Our projects use CMake as the main build configuration tool. Building a project using CMake is a two-stage process. First, CMake needs to be *configured* by calling

```shell 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

This tells CMake to search the current directory `.` (passed via `-S`) for a *CMakeLists.txt* file and process it into a directory `build` (passed via `-B`). The flag `-DCMAKE_BUILD_TYPE=Release` tells CMake to configure a *Release* build (
as opposed to, e.g., a *Debug* build).

After configuring with CMake, the project can be built by calling

```shell
 cmake --build build --config Release
```

This tries to build the project in the `build` directory (passed via `--build`). Some operating systems and developer environments explicitly require a configuration to be set, which is why the `--config` flag is also passed to the build
command. The flag `--parallel <NUMBER_OF_THREADS>` may be added to trigger a parallel build.

Building the project this way generates

- the main library `libsyrec.a` (Unix) / `syrec.lib` (Windows) in the `build/src` directory
- a test executable `syrec_test` containing a small set of unit tests in the `build/test` directory (only if `-DBUILD_SYREC_TESTS=ON` is passed to CMake during configuration)

### Building and Extending the Python Bindings

To extend the Python bindings you can locally install the package in edit mode, so that changes in the Python code are instantly available. The following example assumes you have
a [virtual environment](https://docs.python.org/3/library/venv.html) set up and activated.

```commandline
(venv) $ pip install --editable .
```

If you change parts of the C++ code, you have to run this command again to make the changes visible in Python.

In order for your IDE to pick up the bindings code, you might have to set the `-DBINDINGS=ON` CMake configuration option.

## Reference

If you use our tool for your research, we will be thankful if you refer to it by citing the appropriate publication:

<details open>
<summary>[1] Robert Wille, Majid Haghparast, Smaran Adarsh, and Tanmay M. "Towards HDL-based Synthesis of Reversible Circuits with No Additional Lines". In International Conference on Computer Aided Design (ICCAD), 2019</summary>

```bibtex
@inproceedings{wille2019towardsHDLsynthesis,
    author = {Wille, Robert and Haghparast, Majid and Adarsh, Smaran and M, Tanmay},
    title = {Towards HDL-based Synthesis of Reversible Circuits with No Additional Lines},
    booktitle = {International Conference on Computer Aided Design},
    year = {2019}
}
```

</details>
