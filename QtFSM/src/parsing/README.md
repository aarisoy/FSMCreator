# Parsing Module

The **Parsing** module is responsible for reading generated **C++ config** and reconstructing the FSM model (Reverse Engineering).

## Overview

`CodeParser` performs a strict parse of the C++ config format emitted by `CodeGenerator`. It extracts:

- `cfg.initial`
- `cfg.states["..."] = StateConfig{ ... }` entries
- Transition lists inside each state

This ensures round-trip stability between the visual model and the generated config.

## Notes

Lower-level legacy parsing helpers (`Lexer`, `CppParser`, `ModelBuilder`) remain in the codebase for experimentation and diagnostics, but the application now parses only the config-based format.
