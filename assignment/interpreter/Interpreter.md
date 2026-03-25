## IFJcode25 Target Code Interpreter

The executable interpreter can be downloaded from the course files section:  
**Interpret cílového kódu IFJcode25**

The IFJcode25 interpreter is also available on the Merlin server:

```
/pub/courses/ifj/ic25int/linux/ic25int

```

The interpreter is also available as a container (Docker / Podman, x86_64 only):

```
docker run -v /path/to/program.ifjcode:/app/prog docker.io/leondryaso/ic25int:latest prog

```

Examples written in IFJcode25 (not generated, demonstration only) are available in the course materials.

---

## Command line parameters

| Parameter | Description |
|----------|------------|
| `--help` | Prints basic help for using the interpreter |
| `--verbose` | Enables debug output and more detailed information |
| `--silent` | Disables debug output of BREAK and DPRINT instructions on stderr |

---

## Known properties and limitations

- The `int` type in IFJcode25 is **64-bit**.  
- Evaluation tests assume this behavior.  
- Integer overflow does **not** produce an error.

---

## Visual Studio Code support

A Visual Studio Code extension implementing syntax highlighting and execution support for IFJcodeXX languages is available (by Ondřej Krejčí, at https://marketplace.visualstudio.com/items?itemName=okrejci.ippc-ifjc)
It recognizes files with extensions `.ifjc`, `.ifjcode`, `.ifjc25` and allows running and stepping through programs.  
The extension has been updated to support **IFJcode25**.

---

## Cost of IFJcode25 interpretation

These costs are relevant only for the performance contest.

### Memory access cost

| Cost | Memory access type |
|------|-------------------|
| 1 | Literal / constant written directly in IFJcode25 |
| 2 | Access to value on data stack |
| 4 | Access to variable in frame (LF / GF / TF) |

---

### Instruction cost (without memory access)

| Cost | Instructions |
|------|-------------|
| 0 | LABEL, BREAK, DPRINT, GROOT |
| 1 | MOVE, DEFVAR, PUSHS, POPS, INT2FLOATS, FLOAT2INTS, FLOAT2R2EINTS, FLOAT2R2OINTS, INT2CHARS, STRI2INTS, FLOAT2STRS, INT2STRS, JUMPIFEQS, JUMPIFNEQS, TYPES, ISINTS |
| 2 | PUSHFRAME, POPFRAME, CREATEFRAME, CLEARS, INT2FLOAT, FLOAT2INT, FLOAT2R2EINT, FLOAT2R2OINT, INT2CHAR, STRI2INT, FLOAT2STR, INT2STR, JUMP, JUMPIFEQ, JUMPIFNEQ, TYPE, ISINT, EXIT |
| 3 | ADDS, SUBS, MULS, DIVS, IDIVS, LTS, GTS, EQS, ANDS, ORS, NOTS |
| 4 | ADD, SUB, MUL, DIV, IDIV, LT, GT, EQ, AND, OR, NOT, READ, WRITE, CONCAT, STRLEN, GETCHAR, SETCHAR |
| 5 | CALL, RETURN |

Note: For `DPRINT`, operand loading is not counted in the final statistics.

---

## Example cost calculation (without data stack)

```
.IFJcode25          # ins   op1   op2   op3   TOTAL
DEFVAR GF@a         # 1     4              = 5
DEFVAR GF@b         # 1     4              = 5
DEFVAR GF@c         # 1     4              = 5
MOVE GF@a int@10    # 1     4     1        = 6
MOVE GF@b int@5     # 1     4     1        = 6
ADD GF@c GF@a GF@b  # 4     4     4     4  = 16
WRITE GF@c          # 4     4              = 8
----------------------------------------------

TOTAL                                   = 51
```

---

## Example cost calculation (with data stack)

```
.IFJcode25          # ins   op1   op2   op3   TOTAL
PUSHS int@10        # 3     1              = 4
PUSHS int@5         # 3     1              = 4
ADDS                # 3     2     2     2  = 9
DEFVAR GF@res       # 1     4              = 5
POPS GF@res         # 1     4     2        = 7
WRITE GF@res        # 4     4              = 8
----------------------------------------------

TOTAL                                   = 37
```
