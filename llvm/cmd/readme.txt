LLVM Command Guide

https://llvm.org/docs/CommandGuide/index.html
----------------------------------------------------------
The following documents are command descriptions for all of the LLVM tools. 
These pages describe how to use the LLVM commands and what their options are. 
Note that these pages do not describe all of the options available for all 
tools. To get a complete listing, pass the --help (general options) or 
--help-hidden (general and debugging options) arguments to the tool you are 
interested in.


Basic Commands
-------------------------------------------------------
dsymutil - manipulate archived DWARF debug symbol files
llc - LLVM static compiler
lli - directly execute programs from LLVM bitcode
llvm-as - LLVM assembler
llvm-config - Print LLVM compilation options
llvm-cov - emit coverage information
llvm-cxxmap - Mangled name remapping tool
llvm-diff - LLVM structural ‘diff’
llvm-dis - LLVM disassembler
llvm-dwarfdump - dump and verify DWARF debug information
llvm-lib - LLVM lib.exe compatible library tool
llvm-libtool-darwin - LLVM tool for creating libraries for Darwin
llvm-link - LLVM bitcode linker
llvm-lipo - LLVM tool for manipulating universal binaries
llvm-mca - LLVM Machine Code Analyzer
llvm-profdata - Profile data tool
llvm-readobj - LLVM Object Reader
llvm-stress - generate random .ll files
llvm-symbolizer - convert addresses into source code locations
opt - LLVM optimizer


GNU binutils replacements
--------------------------------------------------------
llvm-addr2line - a drop-in replacement for addr2line
llvm-ar - LLVM archiver
llvm-cxxfilt - LLVM symbol name demangler
llvm-install-name-tool - LLVM tool for manipulating install-names and rpaths
llvm-nm - list LLVM bitcode and object file’s symbol table
llvm-objcopy - object copying and editing tool
llvm-objdump - LLVM’s object file dumper
llvm-ranlib - generates an archive index
llvm-readelf - GNU-style LLVM Object Reader
llvm-size - print size information
llvm-strings - print strings
llvm-strip - object stripping tool


Debugging Tools
--------------------------------------------------------
bugpoint - automatic test case reduction tool
llvm-extract - extract a function from an LLVM module
llvm-bcanalyzer - LLVM bitcode analyzer


Developer Tools
--------------------------------------------------------
FileCheck - Flexible pattern matching file verifier
xxx-tblgen - Target Description to C++ Code
lit - LLVM Integrated Tester
llvm-exegesis - LLVM Machine Instruction Benchmark
llvm-locstats - calculate statistics on DWARF debug location
llvm-pdbutil - PDB File forensics and diagnostics
llvm-profgen - LLVM SPGO profile generation tool
