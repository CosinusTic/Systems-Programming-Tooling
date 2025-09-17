# MY_DBS

## WELCOME
This project gathers 3 systems programming tooling projects programmed in C that were made at EPITA in 2024 targeting ELF x64.
- my_db: a mini debugger similar to GDB implementing breakpoint management, continue, registers, and x|d|u dumps
- my_nm: lists symbols from ELF x64 object files (nm clone)
- my_strace: trace system calls and signals from an object file (ELF x64)

## PROJECT STRUCTURE

### Tree
```
├── my_db
│   ├── Makefile
│   ├── src
│   └── test
├── my_nm
│   ├── Makefile
│   ├── my_nm
│   ├── src
│   └── test
├── my_strace
│   ├── Makefile
│   ├── my_strace
│   ├── parsing
│   ├── src
│   └── test
├── README.md
└── TODO.md
```


## Setup
Everything is ready-to-use, simple navigate to sub-directories and call `make` or `make all` in order to build the projects, then refer to sub-readmes to know how to test them.

### AUTHORS
- Nathan Delmarche
