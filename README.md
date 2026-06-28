# TCC_Pro

TCC_Pro is a modern, educational C compiler written in C++17. It is a complete rewrite of an earlier C-based compiler (`arm_c_compiler`), designed to demonstrate clean architectural principles and modern C++ idioms.

## Architecture

TCC_Pro implements a classic 5-phase compilation pipeline:
1. **Lexical Analysis:** Maps source text to a token stream.
2. **Parsing:** Recursive-descent parser generating a typed Abstract Syntax Tree (AST).
3. **Semantic Analysis:** Validates types, scopes, and semantics using the Visitor pattern.
4. **IR Generation:** Lowers the AST into an intermediate representation (Basic Blocks and IR Instructions).
5. **Code Generation:** Emits target assembly (currently x86 Linux) via a swappable `Backend` abstraction.

### Key Design Goals

* **Zero Global State:** All compiler state is owned by a central `CompilerContext` object, making the pipeline re-entrant and highly testable.
* **Visitor Pattern:** AST traversal (semantic checking, IR generation, dumping) is cleanly separated from the data structures via a virtual `Visitor` interface.
* **Abstract Backend Seam:** A clean `Backend` interface and factory pattern make adding new target architectures straightforward.
* **RAII & Arena Allocation:** Memory management relies on modern C++ principles and arena allocators to prevent leaks and dangling pointers.

## Building

TCC_Pro uses CMake. To build the project:

```bash
mkdir build
cd build
cmake ..
make
```

## Testing

The project includes an equivalence test harness. Run the scripts in the `tests/` directory to compile and verify test cases against expected outputs.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
