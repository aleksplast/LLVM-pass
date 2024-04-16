# LLVM-pass
## Overview
This project is an educational project about LLVM Pass. Developed pass dumps static and dynamic info.

In static it collects:
  - Basic Block ID
  - Function, that BB belongs to
  - Basic Block's successors
  - Instructions inside Basic Block, their Uses and IDs

Result will be `<filename>.pcno` in following format:
```
BBId
Number of Instructions
Function name
Number of Successors
Successor1
...
SuccessorN
{
Instruction1Id NumUses
BBUses1 Uses1
...
BBUsesN UsesN
Instruction1
...
InstructionNId
...
InstructionN
}
NEXT BB INFO
...
```

In dynamic it dumps:
  - Executing Basic Block
  - Result of the Binary Operation (mul, add, sub, div e.t.c)

Result will be `<filename>.pcda` in following format
```
exec BBId
binop BBId InstrId Result
...
```

Then, you can collect and visualize info, using visualizer. Visualizer will paint hot basic blocks in orange colors and cold basic blocks in blue colors. (Basic Block is treated as hot if it was executed more than average number of times). Visualizer will also print average result of binary operation after them with `=`. It will also show contol flow with solid black edges and data flow with dashed edges.

## Build

### Requirements
1. LLVM package (Ubuntu: sudo apt install llvm)
2. CMake version 3.0 or higher
3. graphviz

### Build
```
mkdir build && cd build
cmake ..
make
```

It will generate `libPass.so` - plugin for clang with our Pass. It will also generate `./visualizer` for example usage.

### Usage

1. Generate LLVM IR file

`clang -Xclang -load -Xclang ./libPass.so <src_file> -emit-llvm -S -o <out_name>.ll -flegacy-pass-manager`

2. Generate executable

  `clang -Xclang -load -Xclang ./libPass.so <src_file> ../pass/log.c -flegacy-pass-manager`

3. Run binary
   `./<binary_name>`

4. Generate DOT file

   `./visualizer <src_file>.pcno <src_file>.pcda`

   (this will create `cfg.dot`, if you want to specify output file, run `./visualizer <src_file>.pcno <src_file>.pcda <out_file>`)

5. Generate PNG of CFG

   `dot -Tpng <out_file>.dot -o <out>.png`

## Examples

### Simple Programm
Here is example of CFG for this simple programm:
```
int main() {
    int res = 1 + 1;

    if (res) {
        res = res * 2 + 3 * (res + 2);
    } else {
        res = 2 + 4 * 7;
    }

    return 0;
}
```

CFG:
<img align="center" src="https://github.com/aleksplast/LLVM-pass/assets/111467660/b03907bd-2915-493f-9ea8-2182fb4e7671">

### Factorial
Source file you can find here: examples/factorial.c

CFG:
<img align="center" src="https://github.com/aleksplast/LLVM-pass/assets/111467660/766db093-5bef-4786-aefd-870e90ecad85">

### Something harder
Programm, that calculates maximum subarray from given array you can find here: examples/max_subarray.cpp

CFG for unoptimized programm (-O0):
<img align="center" src="https://github.com/aleksplast/LLVM-pass/assets/111467660/613b655f-f582-44ef-8d56-c83947599a3e">

CFG for optimized programm (-O2):
<img align="center" src="https://github.com/aleksplast/LLVM-pass/assets/111467660/8cf6c9f3-4257-4fd5-a26c-9d9869701a85">

