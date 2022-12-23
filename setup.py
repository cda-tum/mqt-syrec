import os
import re
import subprocess
import sys
from pathlib import Path

from setuptools import Extension, find_namespace_packages, setup
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def build_extension(self, ext):
        from setuptools_scm import get_version

        version = get_version(root=".", relative_to=__file__)

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        # required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")
        cfg = "Debug" if self.debug else "Release"

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DSYREC_VERSION_INFO={version}",
            f"-DCMAKE_BUILD_TYPE={cfg}",
            "-DBINDINGS=ON",
        ]
        build_args = []

        if self.compiler.compiler_type != "msvc":
            if not cmake_generator:
                cmake_args += ["-GNinja"]
        else:
            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})
            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})
            # Convert distutils Windows platform specifiers to CMake -A arguments
            plat_to_cmake = {
                "win32": "Win32",
                "win-amd64": "x64",
                "win-arm32": "ARM",
                "win-arm64": "ARM64",
            }
            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", plat_to_cmake[self.plat_name]]
            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"]
                build_args += ["--config", cfg]

            # cross-compile support for macOS - respect ARCHFLAGS if set
        if sys.platform.startswith("darwin"):
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

            # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        if sys.platform == "win32":
            cmake_args += ["-T", "ClangCl"]

        build_dir = Path(self.build_temp)
        build_dir.mkdir(parents=True, exist_ok=True)
        try:
            Path(build_dir / "CMakeCache.txt").unlink()
        except FileNotFoundError:
            # if the file doesn't exist, it's probably a fresh build
            pass

        subprocess.check_call(["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp)
        subprocess.check_call(
            ["cmake", "--build", ".", "--target", ext.name.split(".")[-1]] + build_args,
            cwd=self.build_temp,
        )


README_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)), "README.md")
with open(README_PATH) as readme_file:
    README = readme_file.read()

setup(
    name="mqt.syrec",
    author="Smaran Adarsh",
    author_email="smaran.adarsh@tum.de",
    description="SyReC - A Programming Language for Synthesis of Reversible Circuits",
    long_description=README,
    long_description_content_type="text/markdown",
    python_requires=">=3.7",
    license="MIT",
    # url="",
    ext_modules=[CMakeExtension("mqt.syrec.pysyrec")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    packages=find_namespace_packages(include=["mqt.*"]),
    extras_require={
        "test": ["pytest>=7"],
        "coverage": ["pytest-cov[toml]"],
        "docs": [
            "sphinx>=5.1.1",
            "sphinx-rtd-theme",
            "sphinxcontrib-bibtex~=2.5",
            "sphinx-copybutton",
            "sphinx-hoverxref",
            "sphinxext.opengraph",
            "sphinx_rtd_dark_mode",
            "pybtex>=0.24",
            "importlib_metadata>=3.6; python_version < '3.10'",
        ],
        "dev": ["mqt.syrec[test, coverage, docs]"],
    },
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: C++",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
        "Intended Audience :: Science/Research",
        "Natural Language :: English",
        "Topic :: Scientific/Engineering :: Electronic Design Automation (EDA)",
    ],
    keywords="MQT reversible_computing synthesis",
    project_urls={
        "Source": "https://github.com/cda-tum/syrec/",
        "Tracker": "https://github.com/cda-tum/syrec/issues",
        "Documentation": "https://syrec.readthedocs.io",
    },
    install_requires=[
        "PyQt6>=6.2.3,<6.5.0",
    ],
    entry_points={
        "console_scripts": ["syrec-editor=mqt.syrec.syrec_editor:main"],
    },
)
