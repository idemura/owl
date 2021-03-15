# The Owl Programming language

This is Owl programming language compiler. The goal of this project is mainly research at this point. Owl
is unmanaged, statically typed procedural language that is very close to C and interoperates with it.
Compared to C, it will offer:
* Type system with clear separation of value and reference types (allocated on the heap)
* Generics (aka templates, but simple and elegant)
* Modules and interfaces

It will include automatic variable type deduction, interesting move/copy semantics. Currently,
it compiles into C code, but going forward, it is planned to generate real binary code with LLVM.

Author: Igor Demura
