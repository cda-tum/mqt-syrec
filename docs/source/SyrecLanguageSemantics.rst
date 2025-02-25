SyReC language semantics
========================
The basic semantics of the SyReC language were defined in :cite:p:`wille2016syrec` and :footcite:p:`wille2008revlib` but are extended in the implemented SyReC parser with a detailed description provided on this page.
We will start by defining the semantics for a SyReC program and its defined modules and are assuming that all entities of the SyReC program are defined in the processed .syrec file.

Modules
-------
- The SyReC specification :footcite:p:`wille2008revlib` (v2.0.1, section 2.1) states that in a well-formed SyReC program, the entry point of the program is either explicitly defined by a module with an identifier 'main' or implicitly chosen as the last defined module of the program.

 Additionally, the SyReC parser supports module overloading, i.e. the definition of modules sharing the same identifier while the structure of the parameters differ, for all modules with an identifier different than 'main'. The signature of two modules (module identifier + parameters) :math:`m_1` and :math:`m_2` is considered to be equal if the following conditions hold:
    - The identifier of the modules match
    - The number of parameters is equal (assumed to be equal to an arbitary but fixed value n)
    - For each parameter at the same index :math:`i: 0 \leq i < n` in the sequence of formal parameters of both modules the following holds:
        - The type of parameter :math:`p_i` of module :math:`m_1` allows for an assignment of the parameter at the same position in the formal parameter list of module :math:`m_2` with the following table listing the assignability between the different SyReC variable types:
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            |                                               | **Assigned from variable type**            |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | **Assigned to variable type**                 | *in*  | *inout* | *out* | *wire* | *state* |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | *in*                                          | X     | X       | X     | X      | X       |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | *inout*                                       | o     | X       | X     | X      | ?       |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | *out*                                         | o     | X       | X     | X      | ?       |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | *wire*                                        | o     | X       | X     | X      | ?       |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | *state*                                       | ?     | ?       | ?     | ?      | ?       |
            +-----------------------------------------------+-------+---------+-------+--------+---------+
            | o ... does not allow assignment                                                            |
            | X ... allows assignment                                                                    |
            +--------------------------------------------------------------------------------------------+
        - The number of dimensions match
        - The number of values for each of the defined dimensions match
        - The bitwidth of the parameters match 

 An error will be reported by the SyReC parser if a duplicate module declaration is detected.

- If one omits to specify the dimensionality of a module parameter or variable then it is assumed to consist of only a single dimension storing a single value (*module main(inout a(4)) ...* is equivalent to *module main(inout a[1](4) ...)*)
- If one omits the bitwidth of a module parameter or variable then a :doc:`configurable <library/Settings>` default value is assumed by the parser.
- The variable and parameter names inside of a module must be unique in the latter.
- The maximum supported variable bitwidth is 32.

Statements
----------
Assignments
^^^^^^^^^^^
- To guarantee the reversibility of any assignment, the assigned to variable parts cannot be accessed on the "other" (right hand side of an non-unary assignment or left/right side of a variable swap) of the assignment. While this restriction is applied to all variable accesses on the "other" side of the assignment, the restriction does not apply to variable accesses defined in the dimension access of any variable access. Additionally, the parser can only detect an overlap between two variable (:math:`l_{varA}` and :math:`r_{varA}`) accesses if the following conditions hold (note loop variables in the following checks are not evaluated to their current value):
    - The identifier of the accessed variables match
    - Assuming that :math:`l_{varA}` defined the indices :math:`l_{dimIdxs} = \{l_1, l_2, \dots, l_n\}` in its dimension access while :math:`r_{varA}` accessed the indices :math:`r_{dimIdxs} = \{r_1, r_2, \dots, r_n\}`, an overlap in the :math:`i`-th dimension is detected iff:
        - Both :math:`l_i` and :math:`r_i` evluated to a constant at compile time and :math:`l_i == r_i`
        - An overlap was detected for all indices :math:`j` at positions :math:`0 < j < i` in the dimension accesses
        - Note that only :math:`min(len(l_{dimIdxs}), len(r_{dimIdxs}))` are checked
    - If an overlap in the dimension access was detected, the accessed bitrange of :math:`l_{varA}` represented by the pair (:math:`l_{bitS}`, :math:`l_{bitE}`) and of :math:`r_{varA}` represented by (:math:`r_{bitS}`, :math:`r_{bitE}`) are checked with an overlap being detected iff:
        - All indices of both bitranges evaluated to constants and an overlap between the two ranges is detected.
        - One bit of each variable access evaluated to a constant and their values match.
        - One bit of one variable access evalauted to a constant while both indices of the accessed bit range in the other variable access evalauted to constant, an overlap is reported if the bit range with known bounds overlaps the bit whos value is known.
    
  Note: Out of range index values are not treated differently than values that are in range.
- While an access on the assigned to variable parts is not allowed in certain parts of the assignment as describe above, the handling of overlaps with the assigned to variable parts in the dimension access of a variable access (as shown in the example below) needs to configured in the SyReC parser by the user

  .. code-block:: text

    module main(inout a(4), in b[3](2))
        a[0].1:2 += b[(a[0].0:2 + 2)]

  The reversibility of the assignment shown in the example depends on whether the expression in the dimension access on the right hand side of the assignment can be synthesized in a reversible way which might depend on the uses synthesizer. By default, the SyReC parser will assume
  that such variable accesses are not allowed but can be enabled via a :doc:`flag <library/Settings>` in the parser configuration. The now described restrictions also applies to both sides of a SwapStatement
    
  .. code-block:: text

    module main(inout a(4), in b[3](2))
        b[(a[0].0:2 + 2)] <=> a[0].1:2;
        a[0].1:2 <=> b[(a[0].0:2 + 2)]

Call-/UncallStatements
^^^^^^^^^^^^^^^^^^^^^^
- Call- and UncallStatements can reference a module for which no matching module signature was processed by the parser at the current position in the SyReC program.
- The SyReC language inherits the following semantics for the Call/UncallStatements from its predecessor language Janus :footcite:p:`yokoyama2007janus`, CallStatements will perform a sequential execution of the called module starting from the first statement in the module body while an UncallStatement will execute the statements in the called module starting from the last statement backwards.
- Recursive module call are allowed by the SyReC parser and it is the responsibility of the developer of the SyReC program to prevent an infinite recursion. However, calls to the either explicitly or implicitly defined main mdoule of the current SyReC program are not allowed.
- While the SyReC parser allows the user to use a variable multiple times as an argument for a module call/uncall, for now it is the responsibility of the user to prevent non-reversible assignments in the called module due to an access on the assigned to variable parts with an invalid access shown in the following example
 
  .. code-block:: text

    module swap(inout left(4), inout right(4))
        left <=> right

    module main(inout a(4))
        // Call will result in access on assigned to variables parts 
        // on both sides of SwapStatement (a <=> a)
        call swap(a, a) 

ForStatement
^^^^^^^^^^^^
- The initial value of a loop variable can be used in the initialization of the iteration ranges 'end' and 'stepsize' value

  .. code-block:: text

    module main(...) 
        for $i = 0 to ($i + 1) step ($i + 2) do 
            ... 
        rof

  which is equivalent to 

  .. code-block:: text

    module main(...) 
        for $i = 0 to 1 step 2 do 
            ... 
        rof

- The identifier of a loop variable (excluding the dollar sign prefix) is allowed to be equal to the one of another variable as long as the latter is not a loop variable defined in a parent loop

  .. code-block:: text

    module main(inout a(4), in i(2))
        for $i = 0 to (#a - 1) do 
            a.0:1 += (i + $i)
        rof

- Due to the requirement that the number of iterations performed by a ForStatement is known at compile time, assignments to loop variables are forbidden.
- If the step size of a ForStatement is not defined it is assumed to equal to 1.
- If the optional second iteration range component in a ForStatement is not defined, it is assumed to be equal to 0

  .. code-block:: text

    module main(inout a(4))
        for (#a - 1) do 
            --= a
        rof

  is equivalent to

  .. code-block:: text

    module main(inout a(4))
        for (#a - 1) to 0 step 1 do 
            --= a
        rof

IfStatement
^^^^^^^^^^^
- The guard condition and its matching counterpart called the closing guard condition (defined in the fi component of the IfStatement) need to match exactly (i.e. an evaluation of the closing guard condition to the same expression as the guard condition is not considered as equal if the former did contain additional symbols). An example for such a missmatch is the following

  .. code-block:: text

    module main(inout a(4), in b(2))
        if ((a.0:1 + b) * 2) then
            skip
        else
            skip
        // Despite the simplified closing guard condition evaluating to the same 
        // expression as the guard condition, the two expressions are not 
        // considered as equal due to the additional symbol on the un-
        // optimized version of the closing guard condition
        fi ((a.0:1 + b) * #b)

VariableAccess
--------------
- All indices defined in the dimension or bit/bitrange access of a variable access are zero based.
- The dimension access can be omitted for variables with a single dimension containing only a single value (i.e. module main(inout a(4)) ++= a).
- Checking whether the value of an index of a variable access is within range of the defined bounds of the referenced variable is only performed if the value of the index evaluates to a constant at compile time.
- The expected operand bitwidth for the operands of an expression defined in the dimension access is set based on the data of the expression and not inherited from the expression enclosing the variable access and reset after the expression of the dimension access was processed. Assuming that the expression of the first accessed dimension of the variable access on the right hand side of the assignment is processed

  .. code-block:: text

    module main(inout a[2](4), in c[2][3](4), in b(2))
      a[0].1:2 += c[(b.0 + 2)][a[1]].0:1

  The expected operand bitwidth set via the variable access on the left hand side of the assignment has a length of 2 which is satisfied by the variable access on the right hand side while the expected operand bitwidth of the operands in the expression of the first dimension of the variable c is expected to have a length of 1 while for the second dimension is must have a length of 4.
- If the accessed bit/bitrange is omitted an access on the full bitwidth of the referenced variable is assumed.
- The SyReC parser supports bitrange accesses where the start index is larger than the end or vice versa (while equal indicies are also supported)

  .. code-block:: text

    module main(inout a(4))
        ++= a.0:2;
        --= a.2:0

Expressions
-----------
- Currently UnaryExpressions are not supported by the SyReC parser.
- Expressions with constant operands are evaluated at compile time.
- Arithmetic and logical simplifications are applied at compile time by default (i.e. a simplification of the expression (a + b) * 0 to 0).
- All operands of an expression must have the same bitwidth (excluding constant integers which are truncated to the expected bitwidth using the :doc:`configured truncation operation <library/Settings>`), with the parser using the first bitrange with known bounds as the reference bitwidth (if such an access exists in the operands) while a bit access with an unknown value for its index will cause the expected operand bitwidth to be set to 0 if the latter does not already have a value. 
- All integer constant values are truncated to the expected operand bitwidth if the latter exists for the expression otherwise the values are left unchanged. However, integer constant values defined in the shift amount component of a ShiftExpression are not truncated since the modify the left hand side of the ShiftExpression and "build" the result instead of being an operand to the result itself. 
 
  The following code example will showcase a few examples and assumes that constant integer values are truncated using the modulo operation

  .. code-block:: text
    
    module main(inout a[2](4), in b(2), in c(4))
        // Expected operand bitwidth set by a[0].0:1 to 2
        a[0].0:1 += (b + 4);                            
        for $i = 0 to (#a - 1) do 
            // Expected operand bitwidth set by a[(b + 2) + 6].$i to 1
            a[(b + 2) + 5].$i += (c.$i + b.0) << 2;     
            // Expected operand bitwidth set by b.2:0 to 3
            a[1].0:($i + 2) += (b.2:0 + 5);              
            // Expected operand bitwidth set by a[0].1:2 to 2
            a[0].1:2 += (((b << 4) + 2) << 1)           
        rof

  The SyReC program above is transformed to

  .. code-block:: text

    module main(inout a[2](4), in b(2), in c(4))
        // 4 MOD 2 = 0 => simplification of expression (b + 0) to 0
        a[0].0:1 += b;                                  
        for $i = 0 to (#a - 1) do 
            // 4 MOD 1 = 0 causes simplification of right hand side expression
            // Note that the expression ((b + 2) + 6) uses a separate expected 
            // operand bitwidth of 2 and is simplified to (b + 1)
            a[(b + 1)].$i += (c.$i + b.0) + 4;     
            // 5 MOD 2 => 1
            a[1].0:($i + 2) += (b.2:0 + 1)              
            // Expected operand bitwidth of 2 causes simplification of (b << 4) to 0 
            // since shift amount is larger than expected bitwidth
            // Remaining expression 2 << 1 evaluated to 4 => 4 MOD 2 = 0
            a[0].1:2 += 0           
        rof

.. rubric:: References
.. footbibliography::