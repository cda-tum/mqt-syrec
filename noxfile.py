from __future__ import annotations

import os

import nox
from nox.sessions import Session

nox.options.sessions = ["lint", "tests"]

PYTHON_ALL_VERSIONS = ["3.7", "3.8", "3.9", "3.10"]

if os.environ.get("CI", None):
    nox.options.error_on_missing_interpreters = True


@nox.session(python=PYTHON_ALL_VERSIONS)
def tests(session: Session) -> None:
    """Run the test suite."""
    session.install("-e", ".[test]")
    session.run("pytest", *session.posargs)


@nox.session
def coverage(session: Session) -> None:
    """Run the test suite and generate a coverage report."""
    session.install("-e", ".[test,coverage]")
    session.run("pytest", "--cov", *session.posargs)


@nox.session
def lint(session: Session) -> None:
    """Lint the Python part of the codebase."""
    session.install("pre-commit")
    session.run("pre-commit", "run", "--all-files", *session.posargs)  # --show-diff-on-failure


@nox.session
def pylint(session: Session) -> None:
    """Run pylint."""

    session.install("pylint")
    session.install("-e", ".")
    session.run("pylint", "mqt.syrec", "--extension-pkg-allow-list=mqt.syrec.pysyrec", *session.posargs)