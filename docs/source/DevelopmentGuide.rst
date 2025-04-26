Development Guide
=================

Ready to contribute to the project? Here is how to set up a local development environment.

Initial Setup
#############

1. Fork the `munich-quantum-toolkit/syrec <https://github.com/munich-quantum-toolkit/syrec>`_ repository on GitHub (see https://docs.github.com/en/get-started/quickstart/fork-a-repo).

2. Clone your fork locally

    .. code-block:: console

        $ git clone git@github.com:your_name_here/syrec mqt-syrec

    .. warning::

        The :code:`--recursive` flag is required to also clone all the required submodules.
        If you happen to forget passing the flag on your initial clone, you can initialize all the submodules by executing :code:`git submodule update --init --recursive` in the main project directory.

3. Change into the project directory

    .. code-block:: console

        $ cd mqt-syrec

4. Create a branch for local development

    .. code-block:: console

        $ git checkout -b name-of-your-bugfix-or-feature

    Now you can make your changes locally.

5. Building the project requires the Boost libraries with a minimum version of *1.71.0*.

    .. tip::

        The Boost libraries can be installed by running the following commands:

        Linux

        .. code-block:: console

                $ sudo apt-get -y install libboost-all-dev

        macOS

        .. code-block:: console

                $ brew install boost

        Windows

                please refer to documentation provided in `Boost library <https://www.boost.org/doc/libs/1_80_0/more/getting_started/windows.html>`_ .

6. (Optional, **highly recommended**) Set up a virtual environment

    .. code-block:: console

        $ python3 -m venv venv
        $ source venv/bin/activate

    .. note::

        If you are using Windows, you can use the following command instead:

        .. code-block:: console

            $ python3 -m venv venv
            $ venv\Scripts\activate.bat

    Ensure that pip, setuptools, and wheel are up to date:

    .. code-block:: console

        (venv) $ pip install --upgrade pip setuptools wheel

7. (Optional, **highly recommended**) Setup `nox <https://nox.thea.codes/en/stable/index.html>`_ to conveniently run many development tasks.

    .. code-block:: console

        (venv) $ pipx install nox

    If you use macOS, then nox is in brew, use :code:`brew install nox`.

    .. note::

        If you do not have `pipx <https://pypa.github.io/pipx/>`_ (pip for applications) installed, you can install it with:

        .. code-block:: console

            (venv) $ pip install pipx
            (venv) $ pipx ensurepath

        If you use macOS, then pipx is in brew, use :code:`brew install pipx`.

8. (Optional) Install `pre-commit <https://pre-commit.com/>`_ to automatically run a set of checks before each commit.

    .. code-block:: console

        (venv) $ pipx install pre-commit
        (venv) $ pre-commit install

    If you use macOS, then pre-commit is in brew, use :code:`brew install pre-commit`.

Working on the core C++ library
###############################

Building the project requires a C++ compiler supporting *C++17*, CMake with a minimum version of *3.24*, and the Boost libraries with a minimum version of *1.71.0*.

    .. note::
        We noticed some issues when compiling with Microsoft's *MSCV* compiler toolchain.
        If you want to start development on this project under Windows, consider using the *clang* compiler toolchain.
        A detailed description of how to set this up can be found `here <https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-160>`_.

Configure and Build
-------------------

Our projects use *CMake* as the main build configuration tool.
Building a project using CMake is a two-stage process.
First, CMake needs to be *configured* by calling

    .. code-block:: console

        $ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_MQT_SYREC_TESTS=ON

This tells CMake to

- search the current directory :code:`.` (passed via :code:`-S`) for a :code:`CMakeLists.txt` file.
- process it into a directory :code:`build` (passed via :code:`-B`).
- the flag :code:`-DCMAKE_BUILD_TYPE=Release` tells CMake to configure a *Release* build (as opposed to, e.g., a *Debug* build).
- the flag :code:`-DBUILD_MQT_SYREC_TESTS=ON` tells CMake to also build the C++ tests.

After configuring with CMake, the project can be built by calling

    .. code-block:: console

        $ cmake --build build --config Release

This tries to build the project in the :code:`build` directory (passed via :code:`--build`).
Some operating systems and development environments explicitly require a configuration to be set, which is why the :code:`--config` flag is also passed to the build command. The flag :code:`--parallel <NUMBER_OF_THREADS>` may be added to trigger a parallel build.

Building the project this way generates

- the main library :code:`libsyrec.a` (Unix) / :code:`syrec.lib` (Windows) in the :code:`build/src` directory
- a test executable :code:`syrec_test` containing unit tests in the :code:`build/test` directory
- the Python bindings library :code:`pysyrec.<...>` in the :code:`build/mqt/syrec` directory

Running C++ Tests
-----------------

We use the `GoogleTest <https://google.github.io/googletest/primer.html>`_ framework for unit testing of the C++ library.
All tests are contained in the :code:`test` directory.
After building the project (as described above), the C++ unit tests can be run by executing the test executable :code:`syrec_test` in the :code:`build/test` directory.

    .. code-block:: console

        [.../build/test] $ ./syrec_test

C++ Code Formatting and Linting
-------------------------------

This project mostly follows the `LLVM Coding Standard <https://llvm.org/docs/CodingStandards.html>`_, which is a set of guidelines for writing C++ code.
To ensure the quality of the code and that it conforms to these guidelines, we use

- `clang-tidy <https://clang.llvm.org/docs/ClangTidy.html>`_ -- a static analysis tool that checks for common mistakes in C++ code, and
- `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`_ -- a tool that automatically formats C++ code according to a given style guide.

Common IDEs like `Visual Studio Code <https://code.visualstudio.com/>`_ or `CLion <https://www.jetbrains.com/clion/>`_ have plugins that can automatically run clang-tidy on the code and automatically format it with clang-format.

- If you are using Visual Studio Code, you can install the `clangd extension <https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd>`_.
- If you are using CLion, you can configure the project to use the :code:`.clang-tidy` and :code:`.clang-format` files in the project root directory.

They will automatically execute clang-tidy on your code and highlight any issues.
In many cases, they also provide quick-fixes for these issues.
Furthermore, they provide a command to automatically format your code according to the given style.

.. note::
    If you want to use clang-tidy from the command line, you first have to configure CMake with :code:`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to generate a compilation database.
    It needs this information to correctly analyze the code.
    After configuring CMake, you can run clang-tidy on a file by calling

    .. code-block:: console

        $ clang-tidy <FILE> -- -I <PATH_TO_INCLUDE_DIRECTORY>

    where :code:`<FILE>` is the file you want to analyze and :code:`<PATH_TO_INCLUDE_DIRECTORY>` is the path to the :code:`include` directory of the project.

Working on the Python module
############################

`Pybind11 <https://pybind11.readthedocs.io/>`_ is used for providing bindings of the C++ core library to Python.
This allows to keep the performance critical parts of the code in C++ while providing a convenient interface for Python users.
All of the bindings code as well as the Python module itself is contained in the :code:`mqt/syrec` directory.

Building the Python module
--------------------------

The recommended way of building the Python module is to perform an editable install using `pip <https://pip.pypa.io/en/stable/>`_.

    .. code-block:: console

        (venv) $ pip install --editable .[dev]

The :code:`--editable` flag ensures that changes in the Python code are instantly available without re-running the command.
The :code:`[dev]` extra makes sure that all dependencies for running the Python tests and building the documentation are available.

.. note::
    When using the :code:`zsh` shell it might be necessary to add double quotes around the :code:`.[dev]` part of the command.

.. warning::
    Do not forget to run the above command again after any changes to the C++ core library or bindings to make the changes available in the Python module.

Running Python Tests
--------------------

The Python part of the code base is tested by unit tests using the `pytest <https://docs.pytest.org/en/latest/>`_ framework.
The corresponding test files can be found in the :code:`test/python` directory.
A :code:`nox` session is provided to conveniently run the Python tests.

    .. code-block:: console

        (venv) $ nox -s tests

This installs all dependencies for running the tests in an isolated environment, builds the Python package, and then runs the tests.

.. note::
    If you don't want to use :code:`nox`, you can also run the tests directly using :code:`pytest`.

    .. code-block:: console

        (venv) $ pytest test/python

Python Code Formatting and Linting
----------------------------------

The Python code is formatted and linted using a collection of `pre-commit hooks <https://pre-commit.com/>`_.
This collection includes:

- `ruff <https://docs.astral.sh/ruff/>`_ -- an extremely fast Python linter and formatter, written in Rust.
- `mypy <http://mypy-lang.org/>`_ -- a static type checker for Python code


You can install the hooks manually by running :code:`pre-commit install` in the project root directory.
The hooks will then be executed automatically when committing changes.

    .. code-block:: console

        (venv) $ pre-commit run -a

Working on the Documentation
############################

The documentation is written in `reStructuredText <https://docutils.sourceforge.io/rst.html>`_ and built using `Sphinx <https://www.sphinx-doc.org/en/master/>`_.
The documentation source files can be found in the :code:`docs/source` directory.
You can build the documentation using the :code:`nox` session :code:`docs`.

    .. code-block:: console

        (venv) $ nox -s docs

This will install all dependencies for building the documentation in an isolated environment, build the Python package, and then build the documentation.

    .. note::
        If you don't want to use :code:`nox`, you can also build the documentation directly using :code:`sphinx-build`.

        .. code-block:: console

            (venv) $ sphinx-build -b html docs/source docs/build

        The docs can then be found in the :code:`docs/build` directory.
