from mqt import syrec

if __name__ == "__main__":
    circ = syrec.circuit()
    prog = syrec.syrec_program()
    read_settings = syrec.read_program_settings()
    p1 = syrec.properties()
    p2 = syrec.properties()
    p3 = syrec.properties()
    p4 = syrec.properties()
    read_settings.default_bitwidth = 32
    error = syrec.py_read_program(prog, "./circuit/simple_add_2.src", read_settings)
    syrec.py_syrec_synthesis(circ, prog, p1, p2)
    my_inp_bitset = syrec.bitset(circ.lines, 6)
    my_out_bitset = syrec.bitset(circ.lines)
    syrec.py_simple_simulation(my_out_bitset, circ, my_inp_bitset, p3, p4)
    gates = circ.num_gates
    lines = circ.lines
    qc = syrec.quantum_costs(circ, circ.lines)
    tc = syrec.transistor_costs(circ, circ.lines)
