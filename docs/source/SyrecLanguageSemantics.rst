SyReC language semantics
========================
The original paper introducing the SyReC language :cite:p:`wille2016syrec` defined the syntax of the language as well as first semantic properties (which were also defined in the RevLib project :footcite:p:`wille2008revlib`). For the remainder of this page, we will assume that the reader has read the relevant sections in both documents. The aim of this document is to provide a detailed description of the SyReC semantics due to the reference documents being vague or failing to define the expected behaviour for certain scenarios.

.. note:: 
  Note that the assumptions made in this document are only valid for this project.

We will start by defining the semantics of the highest-level entity of a SyReC program, a *Module*, and work our way down to the lowest/basic elements of the language:

Module
------
- The RevLib project (v2.0.1, section 2.1) states that the entry point of a well-formed SyReC program is either defined explicitly by a module with an identifier *main* or implicitly chosen as the last defined module of the program.

- Omitting the specification of the dimensionality of any variable (i.e., the number of dimensions and the number of values per dimension) will cause the variable to be considered as a 1D signal storing a single value; thus, the next two declarations are equivalent: *module main(inout a(4)) ...* and *module main(inout a[1](4) ...)*).
- The value of every variable with bitwidth :math:`b` is assumed to be an unsigned integer and thus must be in the range :math:`[0, 2^b]`.
- The maximum supported bitwidth of any variable is equal to 32
- Omitting the bitwidth of a variable will lead to the assumption that its bitwidth is equal to a :doc:`configurable default value <library/Settings>`
- The identifiers of the parameters and variables of a module must be unique in the latter.
- Module overloading (i.e., the definition of a module sharing its identifier with another module while the structure [variable type, dimensionality and bitwidth] of their parameters do not match) is supported for all modules whose identifier is not equal to *main*. Thus, overloading the implicitly defined main module of a SyReC program is possible.

 | The module signatures (module identifier + parameters) of two modules :math:`m_1` and :math:`m_2` are considered to be equal if:

   - The module identifiers match
   - Both modules define the same number of parameters :math:`n`
   - For each parameter :math:`i: 0 \leq i < n` in the sequence of the formal parameters of both modules the following holds:
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

 Two equal module signatures will result in a semantic error being reported.

- The maximum number of values storable in a dimension is equal to :math:`2^32`
- A variable can have at most :math:`2^32` dimensions
- The bitwidth of a variable must be larger than zero
- The number of values for any dimension of a variable must be larger than zero

Statements
----------
Assignments
^^^^^^^^^^^
- To guarantee the reversibility of any assignment, the assigned-to variable parts cannot be accessed on the "other" (right-hand side of a non-unary assignment or left/right side of a variable swap) of the assignment. While this restriction is applied to all VariableAccesses on the "other" side of the assignment, the restriction does not apply to VariableAccess defined in the dimension access of any VariableAccess. The parser can only detect an overlap between two VariableAccesses :math:`l_{varA}` and :math:`r_{varA}` if the following conditions hold (note loop variables in the following checks are not evaluated to their current value):
 
  .. note::
   Loop variables are not evaluated to their current value in the following checks

  - The identifier of the accessed variables match
  - Assuming that :math:`l_{varA}` defined the indices :math:`l_{dimIdxs} = \{l_1, l_2, \dots, l_n\}` in its dimension access while :math:`r_{varA}` accessed the indices :math:`r_{dimIdxs} = \{r_1, r_2, \dots, r_n\}`, an overlap in the :math:`i`-th dimension is detected iff:
        - Both :math:`l_i` and :math:`r_i` evaluate to a constant at compile time and :math:`l_i = r_i`
        - An overlap was detected for all indices :math:`j` at positions :math:`0 < j < i` in the sequence of indices of the dimension accesses

          .. note::
           Note that only :math:`min(len(l_{dimIdxs}), len(r_{dimIdxs}))` indices of the dimension accesses are checked
  - If an overlap in the dimension access was detected, the accessed bitranges of :math:`l_{varA}` (represented by the pair (:math:`l_{bitS}`, :math:`l_{bitE}`)) and of :math:`r_{varA}` (represented by (:math:`r_{bitS}`, :math:`r_{bitE}`)) are checked for an overlap using the following conditions:
        - All indices of both bitranges evaluated to constants and an overlap between the two ranges is detected.
        - A bit of each variable access evaluated to a constant and their values match.
        - A bit of one variable access evaluated to a constant while both indices of the accessed bit range in the bitrange of the other VariableAccess evaluated to constants, an overlap is reported if the bit range with known bounds overlaps said bit.
    
    .. note::
     Out-of-range index values are not treated differently than values that are in range.

- While access on the assigned-to variable parts is not allowed in certain parts of an assignment, as described above, the handling of overlaps with the assigned-to variable parts in the dimension access of a VariableAccess (as shown in the example below) needs special consideration:

  .. code-block:: text

    module main(inout a(4), in b[3](2))
        a[0].1:2 += b[(a[0].0:2 + 2)]

  The reversibility of the assignment depends on whether the expression in the dimension access on the right-hand side of the assignment can be synthesized without leading to an assignment in which a qubit is assigned to itself (i.e. *a[0].1 += a[0].1*). Thus, the user must specify in the :doc:`parser configuration <library/Settings>` whether such accesses are allowed. By default, they are assumed to not be allowed. The same restrictions also apply to both sides of a SwapStatement with the validity of the SwapStatements in the example below depending on the used parser configuration.
    
  .. code-block:: text

    module main(inout a(4), in b[3](2))
        b[(a[0].0:2 + 2)] <=> a[0].1:2;
        a[0].1:2 <=> b[(a[0].0:2 + 2)]

.. note::
 The overlap checks in many cases require that the indices evaluate to constant values at compile time (and will not evaluate the whole value range of loop variables) and in all other cases will not report an overlap. However, the parser not reporting an overlap does not mean the absence of an overlap, as the following example shows:

 .. code-block:: text

   module main(inout a(4))
    for $i = 0 to (#a - 1) do
     a.0 += (a.$i + 2)
    rof

 The parser will not report an overlap in the assignment due to the index of the accessed bit in the VariableAccess on the right-hand side of the assignment not evaluating to a constant at compile time. However, the first iteration of the loop will generate an assignment of the form (*a.0 += (a.0 + 2)*) which cannot be reversed. We recommend also implementing overlap checks in any component using the generated IR representation of the SyReC program that could evaluate the value range of the loop variables (i.e., the logic synthesis process).

Call-/UncallStatements
^^^^^^^^^^^^^^^^^^^^^^
- The current implementation does not require that the module referenced by a Call/UncallStatement was already processed at the current position of the Call/UncallStatement in the SyReC program
- A CallStatement will execute the referenced module starting from the first statement in its module body and ending after the last one was executed while an UncallStatement will perform an execution in the reverse direction with both semantics being inherited from the predecessor language of SyReC (see Janus :footcite:p:`yokoyama2007janus`).
- Recursive module calls are allowed but it is the responsibility of the developer of the SyReC program to prevent an infinite recursion. However, calls to the implicitly or explicitly defined main module of the SyReC program are not allowed.

 .. note::
  Recursive calls to overloads of the implicitly defined main module are possible as long as the last module of the SyReC program is not called.

  .. code-block:: text

   module add(in a(4), in b(4), out c(4))
    c += (a + b)

   // Implicitly defined main module
   module add(in a(8), in b(8), out c(8))
    wire tmp_1(4), tmp_2(4), wire tmp_3(4)

    tmp_1 ^= a.0:3;
    tmp_2 ^= b.0:3;
    call add(tmp_1, tmp_2, tmp_3); // Call OK -> module add(in a(4), ...) called
    c.0:4 ^= tmp_3;
    call add(a, b, c) // Call NOK -> implicit main module called

- While the SyReC parser allows a variable to be used multiple times as a caller argument in a Call/UncallStatement, it is for now the responsibility of the user to prevent non-reversible assignments in the called module. An example of such an invalid access is shown in the following example:

  .. code-block:: text

    module swap(inout left(4), inout right(4))
        left <=> right

    module main(inout a(4))
        // Call will result in access on assigned to variables parts 
        // on both sides of SwapStatement (a <=> a)
        call swap(a, a) 

ForStatement
^^^^^^^^^^^^
- While the SyReC grammar does not require the keyword *do* prior to the body of a ForStatement, the examples shown in both documents use such a keyword. Thus, we assume that this is a typo in the grammar and the *do* keyword is required.

- The initial value of a loop variable can be used in the initialization of the iteration ranges 'end' and 'stepsize' value as shown in the following example

  .. code-block:: text

    module main(...) 
        for $i = 0 to ($i + 1) step ($i + 2) do 
            ... 
        rof

    // Is equivalent to
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

- Due to the requirement that the number of iterations performed by a ForStatement is known at compile time, assignments to loop variables are forbidden
- If the step size of a ForStatement is not defined, it is assumed to equal 1
- If the optional second component of the iteration range in a ForStatement is not defined, it is assumed to be equal 0

  .. code-block:: text

    module main(inout a(4))
        for (#a - 1) do 
            --= a
        rof

   // Is equivalent to
    module main(inout a(4))
        for (#a - 1) to 0 step 1 do 
            --= a
        rof

- Due to the assumption that all variable values can be represented by unsigned integer values, negative step size values are converted to their unsigned value using the C++17 value conversion semantics (see `chapter 7.8 <https://open-std.org/JTC1/SC22/WG21/docs/standards>`_). The same conversion is applied to all negative values determined at compile time.
- Semantic/Syntax errors in the statements of the body of a loop performing no iterations are reported due to the parser not implementing the dead code elimination technique
- The following example will showcase how the iteration range of a SyReC loop is evaluated and could be rewritten as a C loop:

  .. code-block:: text

   module main(inout a(32))
     for $i = 0 to 5 step 2 do 
       ++= a
     rof

   // Is equivalent to the C loop
    unsigned int a = ...;
    for unsigned int i = 0; i <= 5; i += 2 {
      ++= a
    }

  The values of the loop variable *$i* would thus be equal to :math:`0, 2, 4`

- The value of the step size of a ForStatement cannot be defined or evaluate to 0 since this would cause an infinite loop.

IfStatement
^^^^^^^^^^^
- The components of an IfStatement will be referred to as *if <GUARD_CONDITION> then <TRUE_BRANCH> else <FALSE_BRANCH> fi <CLOSING_GUARD_CONDITION*. To be able to identify the matching guard condition for a closing guard condition, the expressions used to define both of these components need to consist of the same character and can thus not evaluate to the same value. An example of an IfStatement violating this rule is the following:

  .. code-block:: text

    module main(inout a(4), in b(2))
        if ((a.0:1 + b) * 2) then
            skip
        else
            skip
        // Despite the simplified closing guard condition evaluating to the same 
        // expression as the guard condition, the two expressions are not 
        // considered as equal due to the difference in the substrings '2' and '#b'
        // between the two expressions
        fi ((a.0:1 + b) * #b)

- Semantic/Syntax errors in any simplified expression of either the guard or closing guard conditions are reported even if the violating expression can be omitted due to the simplification
- Semantic/Syntax errors in the not executed branch of an IfStatement are reported due to the parser not implementing the dead code elimination optimization technique

SwapStatement
^^^^^^^^^^^^^
- Both operands of the swap operation must have the same bitwidth
- Whether the access on the assigned to variable parts in the dimension access of any VariableAccess on the opposite side of the SwapStatement is allowed depends on the configured value of the corresponding flag in the parser configuration (see :doc:`flag <library/Settings>`)
- Assignments to the same variable parts between the two sides of the SwapStatement are not allowed and a semantic error is reported if the parser can detect such an overlap

VariableAccess
--------------
- All indices defined in the dimension or bit/bitrange access of a variable access are zero-based.
- The dimension access can be omitted for variables with a single dimension containing only a single value (i.e., *module main(inout a(4)) ++= a*).
- If the accessed bit/bitrange is omitted an access on the full bitwidth of the referenced variable is assumed.
-  If the value of an index in either the dimension or bit/bitrange access evaluates to a constant at compile time, a validation of whether it is within the defined bounds of the accessed variable is performed and an error is reported in case of an out-of-range value.
- Each expression defining the accessed value of the dimension will use an expected operand bitwidth for its operands that is only valid until the expression was processed. Any outside expected operand bitwidth is ignored (i.e. set in the parent expression of the currently processed VariableAccess). Assuming that the expression of the first accessed dimension of the variable access on the right-hand side of the assignment in the following example is processed

  .. code-block:: text

    module main(inout a[2](4), in c[2][3](4), in b(2))
      a[0].1:2 += c[(b.0 + 2)][a[1]].0:1

  The expected operand bitwidth set by the VariableAccess on the left-hand side of the assignment has a length of 2, which is satisfied by the variable access on the right-hand side.
  However, the expected operand bitwidth of the operands in the expression of the first dimension of the VariableAccess on *c* has a value of 1, while for the second dimension it is equal to 4.

- The SyReC parser does not require that the start index of a bit range access be larger or equal to the end index and thus supports the following index combinations:

  .. code-block:: text

    module main(inout a(4))
        ++= a.0:2;
        --= a.2:0;
        ++= a.0:0

- The number of indices defined in the dimension access component in a VariableAccess must be equal to the number of dimensions of the referenced variable. An example of a valid and invalid DimensionAccesses is shown below:

  .. code-block:: text

   module main(inout a[2][4](4), inout b(2))
     ++= a[0][1]; // OK
     --= b;       // OK
     ++= a[0]     // NOK: Number of accessed dimension does not match number of dimensions of variable 'a'

Expressions
-----------
- **Currently UnaryExpressions are not supported!**.
- Expressions with constant operands are evaluated at compile time.
- Arithmetic and logical simplifications are applied at compile time by default (i.e., will result in a simplification of the expression ((a + b) * 0) to 0). However, semantic/syntax errors in the operands of even the simplified subexpressions are reported with the following code sample showcasing an example:

  .. code-block:: text

   module main(inout a[2](4))
     a[0] += ((a[2] + 2) * (#a - 4))

  While the right-hand side expression of the assignment is simplified to the integer constant *0*, the semantic error causes by the out-of-range index access in the variable access *a[2]* will still be reported.

- All operands of an expression must have the same bitwidth (excluding constant integers that are truncated to the expected bitwidth using the :doc:`configured truncation operation <library/Settings>`), with the parser using the first bitrange with known bounds as the reference bitwidth (if such an access exists in the operands). Any bit access will set the expected operand bitwidth to 1 if the value is not already set.
- All integer constant values are truncated to the expected operand bitwidth, if the latter exists for the expression; otherwise, the values are left unchanged. However, integer constant values defined in the shift amount component of a ShiftExpression are not truncated since they modify the left-hand side of the ShiftExpression and "build" the result instead of being an operand of the overall expression. 
 
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
            // 4 MOD 1 = 0 causes simplification of right-hand side expression
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

  Note that for expressions with constant value operands, the integer truncation is only applied after the expression was evaluated with an example being shown below in which we assume that the truncation is performed using the modulo operation:

  .. code-block:: text

    module main(inout a(4))
     a.0:1 += (2 + (#a + 3))

    // Is equivalent to 
    module main(inout a(4))
     a.0:1 += 1 // 9 MOD 2 = 1

- Expressions with constant integer operands are evaluated using the C++ semantics for unsigned integers.


.. rubric:: References
.. footbibliography::