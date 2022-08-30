from mqt import syrec
import pytest
import json

f_synthesis_no_lines = open('../circuits/circuits_synthesis.json')
data_synthesis_no_lines = json.load(f_synthesis_no_lines)
f_synthesis_no_lines.close()

f_synthesis_add_lines = open('../circuits/circuits_synthesis_add_lines.json')
data_synthesis_add_lines = json.load(f_synthesis_add_lines)
f_synthesis_add_lines.close()

f_simulation_no_lines = open('../circuits/circuits_simulation.json')
data_simulation_no_lines = json.load(f_simulation_no_lines)
f_simulation_no_lines.close()

f_simulation_add_lines = open('../circuits/circuits_simulation_add_lines.json')
data_simulation_add_lines = json.load(f_simulation_add_lines)
f_simulation_add_lines.close()

test_circuit_dir= "../circuits/"
string_src = ".src"


def test_parser():
    for file_name in data_synthesis_no_lines:
        prog = syrec.program()
        error = prog.read(test_circuit_dir+ file_name + string_src)

        assert error == ""


def test_synthesis_no_lines():
    for file_name in data_synthesis_no_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(test_circuit_dir+ file_name + string_src)

        assert error == ""
        assert syrec.syrec_synthesis_no_additional_lines(circ, prog)
        assert data_synthesis_no_lines[file_name]["num_gates"] == circ.num_gates
        assert data_synthesis_no_lines[file_name]["lines"] == circ.lines
        assert data_synthesis_no_lines[file_name]["quantum_costs"] == circ.quantum_cost()
        assert data_synthesis_no_lines[file_name]["transistor_costs"] == circ.transistor_cost()

def test_synthesis_add_lines():
    for file_name in data_synthesis_add_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(test_circuit_dir+ file_name + string_src)

        assert error == ""
        assert syrec.syrec_synthesis_additional_lines(circ, prog)
        assert data_synthesis_add_lines[file_name]["num_gates"] == circ.num_gates
        assert data_synthesis_add_lines[file_name]["lines"] == circ.lines
        assert data_synthesis_add_lines[file_name]["quantum_costs"] == circ.quantum_cost()
        assert data_synthesis_add_lines[file_name]["transistor_costs"] == circ.transistor_cost()


def test_simulation_no_lines():
    for file_name in data_simulation_no_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(test_circuit_dir+ file_name + string_src)

        assert error == ""
        assert syrec.syrec_synthesis_no_additional_lines(circ, prog)

        my_inp_bitset = syrec.bitset(circ.lines)
        my_out_bitset = syrec.bitset(circ.lines)
        set_list = data_simulation_no_lines[file_name]["set_lines"]

        for set_index in set_list:
            my_inp_bitset.set(set_index, True)

        syrec.simple_simulation(my_out_bitset, circ, my_inp_bitset)
        assert data_simulation_no_lines[file_name]["sim_out"] == str(my_out_bitset)



def test_simulation_add_lines():
    for file_name in data_simulation_add_lines:
        circ = syrec.circuit()
        prog = syrec.program()
        error = prog.read(test_circuit_dir+ file_name + string_src)

        assert error == ""
        assert syrec.syrec_synthesis_additional_lines(circ, prog)

        my_inp_bitset = syrec.bitset(circ.lines)
        my_out_bitset = syrec.bitset(circ.lines)
        set_list = data_simulation_add_lines[file_name]["set_lines"]

        for set_index in set_list:
            my_inp_bitset.set(set_index, True)

        syrec.simple_simulation(my_out_bitset, circ, my_inp_bitset)
        assert data_simulation_add_lines[file_name]["sim_out"] == str(my_out_bitset)
