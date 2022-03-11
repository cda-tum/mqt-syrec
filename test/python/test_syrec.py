from mqt import syrec
import pytest
import json

f_synthesis = open('../circuits/circuits_synthesis.json')
data_synthesis = json.load(f_synthesis)
f_synthesis.close() 

f_simulation = open('../circuits/circuits_simulation.json')
data_simulation = json.load(f_simulation)
f_simulation.close() 

test_circuit_dir = "../circuits/"
string_src = ".src"


def test_parser():
    
    for file_name in data_synthesis:
        circ = syrec.circuit()
        prog = syrec.syrec_program()
        read_settings = syrec.read_program_settings()
        read_settings.default_bitwidth = 32
        error = syrec.py_read_program(prog, test_circuit_dir + file_name + string_src, read_settings)
        
        assert error==""
   
   
def test_synthesis():
    for file_name in data_synthesis:
        circ = syrec.circuit()
        prog = syrec.syrec_program()
        read_settings = syrec.read_program_settings()
        read_settings.default_bitwidth = 32
        p1 = syrec.properties()
        p2 = syrec.properties()
        error = syrec.py_read_program(prog, test_circuit_dir + file_name + string_src, read_settings)
        
        assert error==""
        assert True == syrec.py_syrec_synthesis(circ, prog, p1, p2)
        assert data_synthesis[file_name]["num_gates"] == circ.num_gates
        assert data_synthesis[file_name]["lines"] == circ.lines
        assert data_synthesis[file_name]["quantum_costs"] == syrec.quantum_costs(circ, circ.lines)
        assert data_synthesis[file_name]["transistor_costs"] == syrec.transistor_costs(circ, circ.lines)
        
 
def test_simulation():
    for file_name in data_simulation:
        circ = syrec.circuit()
        prog = syrec.syrec_program()
        read_settings = syrec.read_program_settings()
        read_settings.default_bitwidth = 32
        p1 = syrec.properties()
        p2 = syrec.properties()
        error = syrec.py_read_program(prog, test_circuit_dir + file_name + string_src, read_settings)
        
        assert error==""
        assert True == syrec.py_syrec_synthesis(circ, prog, p1, p2)
        
        my_inp_bitset = syrec.bitset(circ.lines)
        my_out_bitset = syrec.bitset(circ.lines)
        set_list = data_simulation[file_name]["set_lines"]
        
        for set_index in set_list:
            my_inp_bitset.set(set_index , True)
        
        assert True == syrec.py_simple_simulation(my_out_bitset, circ, my_inp_bitset, p1, p2)
        assert data_simulation[file_name]["sim_out"] == str(my_out_bitset)
        
