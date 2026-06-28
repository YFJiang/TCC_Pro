# Refactor the UCC ARM C Compiler from C to idiomatic C++

## Context

`arm_c_compiler/` is the UCC educational C compiler: ~14K lines of C in `ucl/` (the
compiler core) plus a ~700-line driver in `driver/`. It is a classic 5-phase compiler —
lex → parse (AST) → semantic check → translate to IR/CFG → emit x86 assembly — written in
a very "C with manual discipline" style:

- **Macro-based inheritance**: `AST_NODE_COMMON`, `SYMBOL_COMMON`, `AST_STATEMENT_COMMON`
  pasted into every struct (`ucl/ast.h`, `ucl/symbol.h`, `ucl/stmt.h`, `ucl/expr.h`).
- **Manual dispatch tables**: function-pointer arrays `ExprTrans[]` (`ucl/tranexpr.c:505`),
  `StmtTrans[]` (`ucl/transtmt.c:583`), and the `OPINFO`/X-macro tables (`ucl/opinfo.h`,
  `ucl/opcode.h`); plus big `switch (categ)` / `switch (op)` blocks and `As*()` cast macros.
- **Hand-rolled containers**: `Vector` (`ucl/vector.[ch]`), chained hash symbol tables
  (`ucl/symbol.c`), ELF-hash string interning (`ucl/str.c`), and `.next`-pointer intrusive
  lists everywhere.
- **Arena memory**: bump allocator over `ProgramHeap`/`FileHeap`/`StringHeap`
  (`ucl/alloc.[ch]`), reset per file; almost nothing is individually freed.
- **Global state + printf errors**: `CurrentHeap`, `ErrorCount`, `Functions`, `Globals`,
  `Level`, `TempNum`, `CurrentToken`, … with `Error()/Fatal()` (`ucl/error.c`, `ucl/ucl.c`).
- **Backend**: only x86 exists (`ucl/x86.c`, `ucl/x86linux.c`, `ucl/x86win32.c`,
  `.tpl` templates). Despite the folder name there is **no ARM backend yet**; `ucl/target.h`
  is the existing (informal) backend seam.

**Goal (per decisions below):** a full, idiomatic modern-C++ rewrite that (a) makes the code
type-safe and maintainable (classes + virtual dispatch + STL + RAII replacing macros, tables,
and globals) and (b) establishes a clean `Backend`/`Target` abstraction so an ARM backend can
be added later by implementing one interface rather than editing the front end.

## Decisions (confirmed with user)

- **Depth:** Full idiomatic rewrite (not a mechanical .c→.cpp port).
- **Motivation:** Maintainability **and** an ARM-ready backend seam, weighted equally.
- **Validation:** **Functional equivalence** — compile test C programs with both the original
  and the rewritten compiler, assemble/link/run, compare stdout + exit code.
- **Language standard:** C++17 (gives `std::variant`, `std::optional`, structured bindings,
  `std::filesystem`, `std::pmr`). C++20 optional if available.

## Scope

- **In scope:** rewrite of the compiler core (`ucl/`) and the driver (`driver/`); a new
  `Backend` interface with the existing x86 codegen ported onto it; a new build system; a
  functional-equivalence test harness.
- **Out of scope (follow-on work):** actually implementing ARM code generation. This refactor
  *designs and exposes* the seam (an `ArmBackend` stub that compiles but is unimplemented);
  real ARM instruction selection/regalloc is a separate effort. Self-hosting/bootstrap (the
  old `make test`) is intentionally dropped — a C compiler cannot compile its own C++ source —
  and replaced by the equivalence harness.

## Target architecture

Rewrite under a single namespace (e.g. `ucc`) with this module mapping. Dispatch moves to the
**Visitor pattern**: AST/IR node classes stay data-only with a virtual `accept(Visitor&)`, and
each pass (checker, folder, translator, dumper) is a `Visitor` subclass — this directly
replaces `ExprTrans[]`/`StmtTrans[]`/`As*()`.

| New module (src/…) | Replaces | Design |
|---|---|---|
| `common/` | `alloc.*`, `error.*`, `ucl.c` globals, `input.h` coord | `CompilerContext` (owns arena, interner, diagnostics, counters), `Diagnostics`, `SourceLocation`, `StringInterner`, `Arena` |
| `lex/` | `lex.*`, `input.*`, `token.h`, `keyword.h` | `Lexer` class (instance state replaces `CurrentToken`/`TokenValue`/`TokenCoord`); `Token` struct |
| `ast/` | `ast.*`, `expr.h`, `stmt.h`, `decl.h` | `Node` base (virtual dtor, `SourceLocation`, `accept(Visitor&)`); `Expr`/`Stmt`/`Decl` subclass hierarchies; `Visitor` base |
| `parse/` | `decl.c`, `expr.c`, `stmt.c`, `ast.c` | `Parser` (recursive descent methods build the AST) |
| `type/` | `type.*` | `Type` hierarchy (`ArithType`, `PointerType`, `ArrayType`, `RecordType`, `FunctionType`, `EnumType`); `TypeContext` interns/canonicalizes; predicates become methods/free fns |
| `symbol/` | `symbol.*` | `Symbol` hierarchy (`VariableSymbol`, `FunctionSymbol`, …); `Scope`/`SymbolTable` over `std::unordered_map<std::string_view,Symbol*>` with nested scopes |
| `sema/` | `declchk.c`, `exprchk.c`, `stmtchk.c`, `fold.c`, `simp.c` | `SemanticChecker` (Visitor), `ConstantFolder`, `Simplifier` |
| `ir/` | `gen.*`, `tranexpr.c`, `transtmt.c`, `flow.c`, `opcode.h`, `opinfo.h` | `IRInst`, `BasicBlock`, `CFGEdge`; `IRGenerator` (Visitor → CFG); `FlowAnalysis`; `OpInfo` as a `constexpr std::array` table |
| `codegen/` | `target.h`, `reg.*`, `emit.c`, `output.*` | abstract `Backend` interface + `RegisterAllocator`; `CodeEmitter` drives the backend |
| `codegen/x86/` | `x86.c`, `x86linux.c`, `x86win32.c`, `.tpl` | `X86Backend` implementing `Backend` (Linux/Win32 variants); templates become data tables |
| `codegen/arm/` | — (new) | `ArmBackend` stub implementing `Backend` (throws "unimplemented") to prove the seam |
| `driver/` | `driver/ucc.c`, `linux.c`, `win32.c` | `Driver` + `Toolchain` interface with `LinuxToolchain`/`Win32Toolchain` |
| `main.cpp` | `ucl.c:main` | constructs `CompilerContext`, runs the pipeline per file |

### Key design choices

- **Memory:** Keep an arena, but as an idiomatic RAII object. Use `std::pmr::monotonic_buffer_resource`
  (or a small `Arena` class) **owned by `CompilerContext`**, reset per file (preserving the old
  `FileHeap` lifetime). Rationale: the AST/IR/CFG graph is inherently cyclic and cross-linked
  (CFG preds/succs, `break`/`continue` targets, `goto`→`Label`, value-numbering) — forcing
  `unique_ptr` ownership would require pervasive raw back-pointers anyway. Arena allocation +
  plain observer pointers is what real compilers (e.g. LLVM's BumpPtrAllocator) do, and the
  context's destructor frees everything (RAII). STL containers (`std::vector`, `std::string`,
  `std::unordered_map`) use the default allocator for their internal storage.
- **Dispatch:** Visitor pattern (see above). The expression `Op` enum and its metadata
  (precedence, name) survive as a `constexpr` `OpInfo` table; expression node *kinds* become
  subclasses (`BinaryExpr`, `UnaryExpr`, `CallExpr`, `MemberExpr`, `CastExpr`, `PrimaryExpr`, …).
- **String interning:** `StringInterner` returning stable `std::string_view` (backed by a
  `std::deque<std::string>`/arena), preserving the pointer-identity comparisons the code relies on.
- **Errors:** A `Diagnostics` engine accumulating errors/warnings with `SourceLocation` (replaces
  global `ErrorCount`/`WarningCount` + `Error()/Fatal()`). Recoverable semantic errors are
  collected, not thrown; exceptions reserved for unrecoverable/internal errors only.
- **No global state:** everything above lives on `CompilerContext`, threaded by reference into
  the pass objects.

## Execution plan

Strategy that keeps work continuously validatable despite being a one-pass full rewrite:
**build the new compiler as a separate binary (`uclxx`) alongside the untouched original
(`ucl`), use the original as the oracle for the equivalence harness, and only retire the C
sources once `uclxx` reaches parity.** Keep originals in `legacy/` (or rely on git history).

1. **Baseline harness first (no source changes).** Stand up the functional-equivalence harness
   (below) against the *original* compiler so we have a green golden baseline before any rewrite.
2. **Bootstrap C++ skeleton + build.** New `src/` tree, `CMakeLists.txt`, namespace, `main.cpp`
   wiring an empty pipeline; `uclxx` builds and runs (no-op).
3. **Common layer:** `Arena`, `StringInterner`, `SourceLocation`, `Diagnostics`, `CompilerContext`.
4. **Front end:** `Token`+`Lexer`; then `ast/` node hierarchy + `Visitor`; then `Parser`.
   Add an `AstDumper` visitor (replacing `dumpast.c`) for debugging parity.
5. **Types & symbols:** `Type` hierarchy + `TypeContext`; `Symbol` hierarchy + `Scope`.
6. **Semantics:** `SemanticChecker`, `ConstantFolder`, `Simplifier`.
7. **IR:** `IRInst`/`BasicBlock`/`CFG`, `IRGenerator`, `FlowAnalysis`, `OpInfo` table.
8. **Backend seam + x86:** define `Backend`/`Target` interface, `RegisterAllocator`, `CodeEmitter`;
   port x86 (Linux first, then Win32) onto it. **Run the equivalence harness against `uclxx`
   here — this is the parity gate.** Add the `ArmBackend` stub to prove the interface.
9. **Driver:** port `ucc` + `Toolchain` to C++.
10. **Cutover & cleanup:** make `uclxx`/the C++ driver the defaults, remove/retire the C sources,
    delete dead macros/headers, update docs (`REAMDE.txt`, `doc/`).

Each step compiles; steps 4–8 are validated against the oracle as soon as the full pipeline can
emit assembly (step 8). Before that, parity is checked structurally via the `AstDumper` and IR
dumper against the originals' `--dump-ast` / `--dump-IR` output.

## Build system

Replace the two `Makefile`s with **CMake (≥3.16), C++17**: one library target for the compiler
core, a `uclxx` executable, a driver executable, the platform split (`LinuxToolchain`/`Win32Backend`)
via `if(WIN32)`/options, and a `ctest` integration that runs the equivalence harness. (A thin
`Makefile` wrapper around CMake can be kept for habit.)

## Verification (functional equivalence)

- **Corpus:** `tests/cases/*.c` — small, self-contained programs that `printf` results and use a
  known exit code, covering arithmetic/promotion, control flow (`if`/`while`/`do`/`for`/`switch`/
  `goto`), functions + recursion, pointers, arrays, structs/unions, enums, typedefs, string
  literals, and floating point. Seed from the `doc/` manual examples and grow as gaps appear.
- **Harness** `tests/run_equivalence.sh` (also wired into `ctest`): for each case, compile with the
  **original** `ucc` and with the **rewritten** driver, then assemble+link+run each and compare
  **stdout + exit code**. Pass = identical. Because the target is 32-bit x86, the harness uses the
  toolchain's 32-bit path (`gcc -m32` / `as`+`ld`); document the `gcc-multilib` dependency.
- **Run** the harness in CI/locally after every milestone (green against the oracle from step 1,
  then against `uclxx` from step 8 onward).
- **Driver smoke test:** end-to-end `ucc hello.c -o hello && ./hello` for both compilers.

## Critical files

- Headers that define the data model to be redesigned: `ucl/ast.h`, `ucl/expr.h`, `ucl/stmt.h`,
  `ucl/decl.h`, `ucl/symbol.h`, `ucl/type.h`, `ucl/gen.h`.
- Dispatch/metadata to replace with Visitor + tables: `ucl/tranexpr.c:505`, `ucl/transtmt.c:583`,
  `ucl/opinfo.h`, `ucl/opcode.h`, `ucl/tokenop.h`.
- Infrastructure to re-home into `CompilerContext`: `ucl/alloc.[ch]`, `ucl/vector.[ch]`,
  `ucl/str.[ch]`, `ucl/error.[ch]`, `ucl/ucl.c`, `ucl/input.h`.
- Backend seam: `ucl/target.h`, `ucl/reg.[ch]`, `ucl/emit.c`, `ucl/output.[ch]`, `ucl/x86*.c`, `*.tpl`.
- Driver: `driver/ucc.c`, `driver/linux.c`, `driver/win32.c`, both `Makefile`s.

## Risks & mitigations

- **One-pass full rewrite of a 14K-line compiler is high-risk.** → Parallel-binary strategy with
  the original as a live oracle; AST/IR dump-diffing before codegen parity; grow the corpus
  aggressively. Internally proceed module-by-module even though the end state is fully idiomatic.
- **32-bit x86 target on a 64-bit host.** → Require/document `gcc-multilib`; consider whether the
  ARM-seam work should also retarget the existing backend — but keep x86-32 as the parity reference.
- **Cyclic ownership in AST/IR/CFG.** → Arena + observer pointers (decided above), not `unique_ptr`.
- **Pointer-identity assumptions** (interned names/types compared with `==`). → Preserve via
  `StringInterner`/`TypeContext` returning canonical pointers.
