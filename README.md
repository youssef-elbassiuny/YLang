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
git clone https://github.com/youssef-elbassiuny/comp.git
cd comp
```

## Complete Workflow

The complete workflow from the `build` directory is:

```bash
cmake -G "Unix Makefiles" ..
cmake --build .
./main ../test/test1.txt
lli output.ll
echo $?
```

## Example Program

Example source code:

```cpp
let x = 10;
let i = 0;

while (i < 5)
{
    x = x + 1;
    i = i + 1;
}

if (x == 15)
{
    let result = x / 3;
    return result;
}
else
{
    return 0;
}
```

Save the program as a `.txt` file and pass it to the compiler:

```bash
./main ../test/test1.txt
```

The compiler generates an LLVM IR file named `output.ll`.

Run the generated LLVM IR using LLVM's interpreter:

```bash
lli output.ll
```

And check the return value:

```bash
echo $?
```

## Supported Features

The compiler currently supports:

- Variables
- Variable assignment
- Arithmetic expressions
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
- Nested scopes
- `return`
- LLVM IR code generation

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
comp/
├── include/
├── scr/
├── test/
├── CMakeLists.txt
└── README.md
```

## Technologies

- C++
- CMake
- LLVM
- LLVM IR
- Linux
- Git

## Status

This project is currently under development.
