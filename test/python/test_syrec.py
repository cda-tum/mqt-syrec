import json
from pathlib import Path

import pytest
from mqt import syrec

test_dir = Path(__file__).resolve().parent.parent
configs_dir = test_dir / "configs"
circuit_dir = test_dir / "circuits"


@pytest.fixture
def data_synthesis_no_lines():
    with open(configs_dir / "circuits_synthesis.json") as f:
        return json.load(f)


@pytest.fixture
def data_synthesis_add_lines():
    with open(configs_dir / "circuits_synthesis_add_lines.json") as f:
        return json.load(f)


@pytest.fixture
def data_simulation_no_lines():
    with open(configs_dir / "circuits_simulation.json") as f:
        return json.load(f)


@pytest.fixture
def data_simulation_add_lines():
    with open(configs_dir / "circuits_simulation_add_lines.json") as f:
        return json.load(f)


def test_parser(data_synthesis_no_lines):
    for file_name in data_synthesis_no_lines:
        prog = syrec.program()
        error = prog.read(str(circuit_dir / (file_name + ".src")))

        assert error == ""


def test_synthesis_no_lines(data_synthesis_no_lines):
    for file_name in data_synthesis_no_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(str(circuit_dir / (file_name + ".src")))

        assert error == ""
        assert syrec.syrec_synthesis_no_additional_lines(circ, prog)
        assert data_synthesis_no_lines[file_name]["num_gates"] == circ.num_gates
        assert data_synthesis_no_lines[file_name]["lines"] == circ.lines
        assert data_synthesis_no_lines[file_name]["quantum_costs"] == circ.quantum_cost()
        assert data_synthesis_no_lines[file_name]["transistor_costs"] == circ.transistor_cost()


def test_synthesis_add_lines(data_synthesis_add_lines):
    for file_name in data_synthesis_add_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(str(circuit_dir / (file_name + ".src")))

        assert error == ""
        assert syrec.syrec_synthesis_additional_lines(circ, prog)
        assert data_synthesis_add_lines[file_name]["num_gates"] == circ.num_gates
        assert data_synthesis_add_lines[file_name]["lines"] == circ.lines
        assert data_synthesis_add_lines[file_name]["quantum_costs"] == circ.quantum_cost()
        assert data_synthesis_add_lines[file_name]["transistor_costs"] == circ.transistor_cost()


def test_simulation_no_lines(data_simulation_no_lines):
    for file_name in data_simulation_no_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(str(circuit_dir / (file_name + ".src")))

        assert error == ""
        assert syrec.syrec_synthesis_no_additional_lines(circ, prog)

        my_inp_bitset = syrec.bitset(circ.lines)
        my_out_bitset = syrec.bitset(circ.lines)
        set_list = data_simulation_no_lines[file_name]["set_lines"]

        for set_index in set_list:
            my_inp_bitset.set(set_index, True)

        syrec.simple_simulation(my_out_bitset, circ, my_inp_bitset)
        assert data_simulation_no_lines[file_name]["sim_out"] == str(my_out_bitset)


def test_simulation_add_lines(data_simulation_add_lines):
    for file_name in data_simulation_add_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(str(circuit_dir / (file_name + ".src")))

        assert error == ""
        assert syrec.syrec_synthesis_additional_lines(circ, prog)

        my_inp_bitset = syrec.bitset(circ.lines)
        my_out_bitset = syrec.bitset(circ.lines)
        set_list = data_simulation_add_lines[file_name]["set_lines"]

        for set_index in set_list:
            my_inp_bitset.set(set_index, True)

        syrec.simple_simulation(my_out_bitset, circ, my_inp_bitset)
        assert data_simulation_add_lines[file_name]["sim_out"] == str(my_out_bitset)
