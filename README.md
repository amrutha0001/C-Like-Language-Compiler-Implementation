# Design and Implementation of a C-like Language Compiler in C

This project is a simple C compiler that implements a lexical analyzer for a subset of the C programming language. It reads multi-line user input, categorizes tokens like keywords, identifiers, constants, operators, and punctuation, and outputs detailed token statistics.

## 📂 Directory Structure

```
Compiler Design/
│
├── codes/                  # C source files for lexical analyzer, parser, and other logic
│   ├── lex.c               # Lexical analyzer implementation
│   ├── parse.c             # Parsing logic
│   ├── run.c               # Code execution logic
│   ├── main.c              # Main entry point
│
├── examples/               # Example input files to test the compiler
│   ├── filename.txt
│   ├── countdigits.txt
│   ├── factorial.txt
│   ├── fibonacci.txt
│   ├── grades.txt
│   ├── largestof3.txt
│   ├── power.txt
│   ├── printeven.txt
│   ├── reverse.txt
│   ├── smallestof3.txt
│   ├── sum.txt
│   ├── swap.txt
│   ├── tables.txt
│   ├── array1.txt
│   ├── array2.txt
│   ├── array3.txt
│
├── obj/                    # Object files directory (Created through commands)
│
├── outputs/                # Outputs directory (Created through commands)
```


## 📌 Features

- **Keyword recognition**: Identifies keywords like `int`, `float`, `if`, `else`, `switch`
- **Identifier recognition**: Supports variables starting with alphabetic characters or underscores
- **Constant recognition**: Identifies numeric constants, including decimals
- **Operator recognition**: Supports arithmetic and logical operators like `+`, `-`, `*`, `/`, `==`, `!=`, `&&`, `||`
- **Punctuation recognition**: Recognizes common punctuation such as `;`, `,`, `{`, `}`, `(`, `)`
- **Finite State Machine (FSM)**: Used for efficient token detection
- **Token classification**: Categorizes tokens and stores them in separate lists
- **Token statistics**: Outputs the number of tokens for each category

## 🚀 Getting Started

### Requirements

- A C compiler (e.g., GCC)
- Basic knowledge of terminal/command-line usage

### Compile and Run

To create `.o` files and compile the project, run the following commands:

```bash
mkdir -p obj
gcc -std=gnu11 -Wall -Werror -c codes/lex.c -o obj/lex.o 
gcc -std=gnu11 -Wall -Werror -c codes/parse.c -o obj/parse.o
gcc -std=gnu11 -Wall -Werror -c codes/run.c -o obj/run.o
gcc -std=gnu11 -Wall -Werror -c codes/main.c -o obj/main.o
gcc -o interpret obj/lex.o obj/parse.o obj/run.o obj/main.o   

<!--To run the compiled code with example input files:-->
./interpret examples/filename.txt
./interpret examples/countdigits.txt
./interpret examples/factorial.txt  
./interpret examples/fibonacci.txt
./interpret examples/grades.txt
./interpret examples/largestof3.txt
./interpret examples/power.txt
./interpret examples/printeven.txt
./interpret examples/reverse.txt
./interpret examples/smallestof3.txt
./interpret examples/sum.txt
./interpret examples/swap.txt
./interpret examples/tables.txt
./interpret examples/array1.txt
./interpret examples/array2.txt
./interpret examples/array3.txt
```

## 📘 Learning Outcomes
Basics of compiler design and lexical analysis

Practical implementation of tokenization and FSM in C

Understanding string manipulation, character classification, and categorizing tokens
