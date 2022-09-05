[![PyPI](https://img.shields.io/pypi/v/mqt.syrec?logo=pypi&style=flat-square)](https://pypi.org/project/mqt.syrec/)
![OS](https://img.shields.io/badge/os-linux%20%7C%20macos%20%7C%20windows-blue?style=flat-square)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![CI](https://img.shields.io/github/workflow/status/cda-tum/syrec/CI?style=flat-square&logo=github&label=c%2B%2B)](https://github.com/cda-tum/syrec/actions/workflows/ci.yml)
[![Bindings](https://img.shields.io/github/workflow/status/cda-tum/syrec/Deploy%20to%20PyPI?style=flat-square&logo=github&label=python)](https://github.com/cda-tum/syrec/actions/workflows/deploy.yml)
[![codecov](https://img.shields.io/codecov/c/github/cda-tum/syrec?style=flat-square&logo=codecov)](https://codecov.io/gh/cda-tum/syrec)

# MQT SyReC - A Programming Language for the Synthesis of Reversible Circuits

A tool for HDL-based synthesis of reversible circuits developed by the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the [Technical University of Munich](https://www.tum.de/).
SyReC is part of the Munich Quantum Toolkit (MQT) and builds upon [our quantum functionality representation (QFR)](https://github.com/cda-tum/qfr).

If you have any questions, feel free to contact us via [quantum.cda@xcit.tum.de](mailto:quantum.cda@xcit.tum.de) or by creating an issue on [GitHub](https://github.com/cda-tum/syrec/issues).

## Getting Started

SyReC is available via [PyPI](https://pypi.org/project/mqt.syrec/) for Linux, macOS, and Windows.

- In order to make the library as easy to use as possible (without compilation), we provide pre-built wheels for most common platforms (64-bit Linux, MacOS, Windows). These can be installed using
  ```bash
  (venv) $ pip install mqt.syrec
  ```
- Once installed, start the SyReC editor GUI by running:
  ```bash
  (venv) $ syrec-editor
  ```

## Reference

SyReC has been developed based on methods proposed in the following papers:

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

If you use our tool for your research, we would appreciate if you cited the appropriate publication.

---

## Documentation for Developers

### System requirements

Building (and running) is continuously tested under Linux, MacOS, and Windows using the [latest available system versions for GitHub Actions](https://github.com/actions/virtual-environments). However, the implementation should be compatible
with any current C++ compiler supporting C++17 and a minimum CMake version of 3.14.

_Disclaimer_: We noticed some issues when compiling with Microsoft's `MSCV` compiler toolchain. If you are developing under Windows, consider using the `clang` compiler toolchain. A detailed description of how to set this up can be
found [here](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-160).

### Setup, Configure, and Build

To start off, clone this repository using

```shell
git clone --recurse-submodules -j8 https://github.com/cda-tum/syrec
```

Note the `--recurse-submodules` flag. It is required to also clone all the required submodules. If you happen to forget passing the flag on your initial clone, you can initialize all the submodules by
executing `git submodule update --init --recursive` in the main project directory.

Our projects use CMake as the main build configuration tool. Building a project using CMake is a two-stage process. First, CMake needs to be _configured_ by calling

```shell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

This tells CMake to search the current directory `.` (passed via `-S`) for a _CMakeLists.txt_ file and process it into a directory `build` (passed via `-B`). The flag `-DCMAKE_BUILD_TYPE=Release` tells CMake to configure a _Release_ build (
as opposed to, e.g., a _Debug_ build).

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
