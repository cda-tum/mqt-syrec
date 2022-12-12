[![PyPI](https://img.shields.io/pypi/v/mqt.syrec?logo=pypi&style=flat-square)](https://pypi.org/project/mqt.syrec/)
![OS](https://img.shields.io/badge/os-linux%20%7C%20macos%20%7C%20windows-blue?style=flat-square)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![CI](https://img.shields.io/github/workflow/status/cda-tum/syrec/CI?style=flat-square&logo=github&label=c%2B%2B)](https://github.com/cda-tum/syrec/actions/workflows/ci.yml)
[![Bindings](https://img.shields.io/github/workflow/status/cda-tum/syrec/Deploy%20to%20PyPI?style=flat-square&logo=github&label=python)](https://github.com/cda-tum/syrec/actions/workflows/deploy.yml)
[![codecov](https://img.shields.io/codecov/c/github/cda-tum/syrec?style=flat-square&logo=codecov)](https://codecov.io/gh/cda-tum/syrec)

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/cda-tum/qmap/main/docs/source/_static/mqt_light.png" width="60%">
    <img src="https://raw.githubusercontent.com/cda-tum/qmap/main/docs/source/_static/mqt_dark.png" width="60%">
  </picture>
  </p>

# SyReC Synthesizer: An MQT Tool for the Synthesis of Reversible Circuits

A tool for HDL-based synthesis of reversible circuits developed by the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the [Technical University of Munich](https://www.tum.de/).
The _SyReC Synthesizer_ is part of the Munich Quantum Toolkit (MQT) and builds upon [our quantum functionality representation (QFR)](https://github.com/cda-tum/qfr).

<p align="center">
  <a href="https://syrec.readthedocs.io/en/latest/">
  <img width=30% src="https://img.shields.io/badge/documentation-blue?style=for-the-badge&logo=read%20the%20docs" alt="Documentation" />
  </a>
</p>

If you have any questions, feel free to contact us via [quantum.cda@xcit.tum.de](mailto:quantum.cda@xcit.tum.de) or by creating an issue on [GitHub](https://github.com/cda-tum/syrec/issues).

## Getting Started

The _SyReC Synthesizer_ is available via [PyPI](https://pypi.org/project/mqt.syrec/) for Linux, macOS, and Windows.

- In order to make the library as easy to use as possible (without compilation), we provide pre-built wheels for most common platforms (64-bit Linux, MacOS, Windows). These can be installed using
  ```bash
  (venv) $ pip install mqt.syrec
  ```
- Once installed, start the _SyReC Synthesizer_ GUI by running:
  ```bash
  (venv) $ syrec-editor
  ```

**Detailed documentation on all available methods, options, and input formats is available at [ReadTheDocs](https://syrec.readthedocs.io/en/latest/).**

## System Requirements and Building

The implementation is compatible with any C++17 compiler and a minimum CMake version of 3.14.
Please refer to the [documentation](https://syrec.readthedocs.io/en/latest/) on how to build the project.

Building (and running) is continuously tested under Linux, macOS, and Windows using the [latest available system versions for GitHub Actions](https://github.com/actions/virtual-environments).

## References

In case you are using _SyReC Synthesizer_ in your work, we would be thankful if you referred to it by citing the following publication:

[[1]](https://www.sciencedirect.com/science/article/pii/S266596382200135X)
S. Adarsh, L. Burgholzer, T. Manjunath and R. Wille. SyReC Synthesizer: An MQT tool for synthesis of reversible circuits. Software Impacts Journal, 2022.
