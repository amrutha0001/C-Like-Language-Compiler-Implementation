# Design and Implementation of a C-like Language Compiler in C

This project is a simplified lexical analyzer built in C that mimics the behavior of a compiler for a subset of the C programming language. It identifies and categorizes tokens such as keywords, identifiers, constants, operators, and punctuation from multiline user input.

## 📌 Features

- Recognizes basic C tokens:  
  - **Keywords**: `int`, `float`, `if`, `else`, `switch`  
  - **Identifiers**: Variables with alphabetic or underscore starts  
  - **Constants**: Numeric values including decimals  
  - **Operators**: Arithmetic and logical operators  
  - **Punctuation**: Braces, commas, semicolons, etc.  
- Uses Finite State Machine (FSM)-based logic for token detection  
- Handles multiline input from the user  
- Classifies and stores each token type separately  
- Displays categorized token counts and their values  

## 📂 File Structure

- `compiler.c` - Main C source file with token recognition logic  
- `README.md` - Project overview and documentation  

## 🚀 Getting Started

### Requirements

- GCC compiler or any standard C compiler  
- Basic knowledge of terminal/command-line

### Compile and Run

```bash
gcc compiler.c -o compiler
./compiler
