# YLang

A small compiler written in C++ that parses a custom programming language and generates LLVM Intermediate Representation (LLVM IR).

## Requirements

Before using the compiler, make sure you have the following installed:

- C++
- CMake
- Make
- LLVM (including `lli`)
- Linux

## Building the Compiler

First, clone the repository:

```bash
git clone https://github.com/youssef-elbassiuny/YLang.git
cd YLang
```

Create and enter the build directory:

```bash
mkdir build
cd build
```

## Complete Workflow

The complete workflow from the `build` directory is:

```bash
cmake -G "Unix Makefiles" ..
cmake --build .
./ylang ../test/test1.txt
lli ylang.ll
echo $?
```

## Example Program

Example source code:

```cpp
fun add(a, b) {
    return a + b;
}

let x = 10;
const y = 5;

let result = add(x, y);

if (result == 15) {
    return result;
}
else {
    return 0;
}
```

Save the program as a `.txt` file and pass it to the compiler:

```bash
./ylang ../test/test1.txt
```

The compiler generates an LLVM IR file named `ylang.ll`.

Run the generated LLVM IR using LLVM's interpreter:

```bash
lli ylang.ll
```

And check the return value:

```bash
echo $?
```

## Supported Features

The compiler currently supports:

- Variables and constants:
  - Variable declarations using `let`
  - Constant declarations using `const let`
  - Variable assignment
  - Constant protection (constants cannot be reassigned)
- Function parameters:
  - Regular parameters
  - Constant parameters using `const`
- Arithmetic expressions:
  - Addition (`+`)
  - Subtraction (`-`)
  - Multiplication (`*`)
  - Division (`/`)
- Parenthesized expressions
- Boolean expressions
- Comparison operators:
  - Less than (`<`)
  - Greater than (`>`)
  - Less than or equal to (`<=`)
  - Greater than or equal to (`>=`)
  - Equality (`==`)
- Conditional statements:
  - `if`
  - `elif`
  - `else`
- `while` loops
- Nested scopes and variable shadowing
- Functions:
  - Function definitions
  - Function parameters
  - Function calls
  - Return values
- `return` statements
- LLVM IR code generation

## Error Handling

YLang includes basic error handling to detect invalid source code during compilation.

The compiler reports errors with their corresponding line numbers to help identify and debug issues.

Current error handling includes:

- **Tokenizer errors:** Detecting invalid or unrecognized characters.
- **Parser errors:** Detecting syntax errors such as missing expressions, missing semicolons, missing closing braces, and invalid statements.
- **Error reporting:** Displaying error messages along with line numbers.
- **Constant protection:** Detecting attempts to reassign constants.

## Current Limitations

- `while` and `if-else` statements are not fully supported inside functions yet.
- The compiler is still under development, and some language features may have limitations.

## Compiler Pipeline

The compiler works through several stages:

```text
Source Code
    |
    v
Tokenizer
    |
    v
Tokens
    |
    v
Parser
    |
    v
AST (Abstract Syntax Tree)
    |
    v
Code Generator
    |
    v
LLVM IR
    |
    v
LLVM Interpreter (lli)
    |
    v
Program Exit Code
```

## Project Structure

```text
YLang/
├── docs/
│   └── grammar.ebnf
├── include/
├── scr/
├── test/
├── CMakeLists.txt
└── README.md
```