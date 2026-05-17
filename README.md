# TSL
Turing Simulated Language (TLS) is a transpiled programming language that simulates the operations of a Turing Machine.  
It consists of two modes: MANUAL mode, in which the TM logic and operations are done manually via commands such as WRITE, L (Left), and R (Right)  
and TRANSITION mode, which uses the formal mathematical defintition of a TM found in many Automata textbooks to automatically accept or reject an input  
based on transition functions.

# Setting Up
## Reequirements
Currently, TSL only works in a terminal environment that supports linux commands. On Windows, common options include MSYS2, UCRT64, or WSL.
## Running
The TSL compiler requires no extra dependencies. So, after cloning the repository, the compiler can be compiled using a command such as:  

g++ compiler.cpp compile  

The 'compile' executable takes as an argument the name of the TSL file you wish to compile (including the extension)  
An example command used to run a TSL program called program.TSL is:  

./compile program.TSL  

followed by:  

./program.TSL <input OPTIONAL>

# Example Programs
This repository contains 5 example programs under the ExampleProgram folder showcasing some of the different features and uses of TSL.  
To run a program from the ExamplePrograms folder, use the command:  

../compile <ProgramName>.TSL  
./<ProgramName>.TSL <input OPTIONAL>

# Documentation
## Basics
TSL simulates the operations of a Turing Machine. Every command in this language is used to operate on a tape which is functionally infinite in both directions.  
The tape is accessed via a head which can be moved left and right and be used to write to the tape. The value pointed at by the head can also be used for control flow.  

In terms of syntax, the language uses newlines as a delimiter between instructions and curly braces to mark code blocks for loops and conditionals.  
The # symbol is used to denote a blank space on the tape. The tape is filled with

## Modes
TSL has two modes. The first mode is Manual execution mode. In Manual mode, the tape head is controlled manually using the regular top to bottom execution and  
control structures used in most other languages. This mode is useful when making transducers, or when you desire to print the contents of the tape to the terminal  
after execution. Manual is the default mode. It is denoted in the program as a Turing Machine state using the MANUAL keyword.
  
The second mode is Transition execution mode. In this mode, the Turing Machine uses predefines states and transition functions to continuosly run until either  
the input is rejected, accepted, or the program switches back to Manual mode. There are only two types of instructions for this mode, however they can be  
combined to create incredibly complex machines.

## Instructions
The following is a list of all instructions available for use in TSL seperated by the mode in which they are used.

### Manual Instructions
R - Moves the tape head one unit right  
L - Moves the tape head one unit left  
WRITE <char> - writes the given symbol to the current location on the tape. Input must be a char  

PRINT - Prints the contents of the tape. Only prints as far as the tape head went in each direction  
ACCEPT - Accepts the input of the program and exits  
REJECT - Rejects the input of the program and exits  

REPEAT <NUMBER> {} - Repeats the code inside of the curly braces <NUMBER> amount of times. If <NUMBER> is not included, this acts as an infinite loop  
WHILE <SYMBOL> {} - Repeats the code inside of the curly braces as long as the tape head is reading the symbol <SYMBOL>  
UNTIL <SYMBOL> {} - Repeats the code inside of the curly braces until the symbol <SYMBOL> is encountered  

IF <SYMBOL> {} - Executes the code inside of the curly braces if the tape head is currently reading the symbol <SYMBOL>  
ELSE {} - Runs the code inside of the curly braces if it immediately follows a failed IF statement  

STATE <NAME> - Transitions the current state to the state with name <NAME>. This sets the program to Transition execution mode

### Transition Instructions
DEF STATE <NAME>: <TYPE>  

This instruction defines a new state with the name <NAME> and the type <TYPE>. Valid types include: N, F, R, A. N denotes a normal state which will not  
accept the input if the machine halts. F denotes a final state which will accept the input if the machine halts. R denotes a rejection state which will  
immediately reject the input regardless of whether or not the machine has halted. A denotes an accepted state which will immediately accept the input  
regardless of whether or not the machine has halted.  

DEF TRANSITION: <CURRENT STATE>, <TAPE SYMBOL>, <PLACE SYMBOL>, <MOVE>, <NEXT STATE>  

This instruction defines a new transition. When in transition execution mode, if the current state is <Current State> and the symbol pointed to by the  
head is <TAPE SYMBOL>, the head will then place <PLACE SYMBOL> on the tape before moving according to the <MOVE> instruction. The current state will then  
be set to <NEXT STATE>. Valid move operations include R and L as defined under Manual Instruction, as well as S, which keeps the head in the same location.