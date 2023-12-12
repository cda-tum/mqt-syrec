"""MQT SyReC library.

This file is part of the MQT SyReC library released under the MIT license.
See README.md or go to https://github.com/cda-tum/syrec for more information.
"""

from __future__ import annotations

from ._version import version as __version__
from .pysyrec import (
    bitset,
    circuit,
    cost_aware_synthesis,
    gate,
    gate_type,
    line_aware_synthesis,
    program,
    properties,
    read_program_settings,
    simple_simulation,
)

__all__ = [
    "bitset",
    "circuit",
    "gate",
    "gate_type",
    "program",
    "properties",
    "read_program_settings",
    "simple_simulation",
    "cost_aware_synthesis",
    "line_aware_synthesis",
    "__version__",
]
