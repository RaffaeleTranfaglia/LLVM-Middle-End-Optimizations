# LLVM-Middle-End-Optimizations
Machine independent passes to optimise LLVM intermediate representation.  

## Local Optimizations
### Installation
In order to run the optimization passes, it is necessary having LLVM 17.0.6 source code installed.  
Official repository [here](https://github.com/llvm/llvm-project).  

The following steps assume that LLVM 17.0.6 has been installed.  
Clone the current repository:
```
git clone https://github.com/RaffaeleTranfaglia/LLVM-Middle-End-Optimizations.git
```
`src/LocalOpts/LocalOpts.cpp` file must be moved to the following directory:  
```
$SRC/llvm/lib/Transforms/Utils
```
Add `LocalOpts.cpp` in `$SRC/llvm/lib/Transforms/Utils/CMakeLists.txt`.  
`src/LocalOpts/LocalOpts.h` file must be moved to the following directory:  
```
$SRC/llvm/include/llvm/Transforms/Utils
```
Replace `SRC/llvm/lib/Passes/PassRegistry.def` with the provided `src/PassRegistry.def`.  
Add the follwing line to `SRC/llvm/lib/Passes/PassBuilder.cpp`:
```
#include "llvm/Transforms/Utils/LocalOpts.h"
```
Where `$SRC` is the source folder of the project.  
To compile the source code:
```
cd $ROOT/BUILD
make opt
make install
```
  
Otherwise, to install the source code already containig the optimized passes' files: [here](https://github.com/Glixes/LLVM_middle_end).

### Introduced Optimizations

#### Constant Folding
Constant Folding execute at compile time operations involving contant values.  
Examples:
- `x = 4 + 9` &#8594; `x = 13 + 0`
- `x = 6 * 2` &#8594; `x = 12 + 0`

Observation:
The assignment of the computed constant is carryed out using an addition of the given constant with zero.  
Afterwards the introduced `add` will furtherly be optimized by the Algebric Identity optimization.

#### Algebraic Identity optimization 
Algebraic Identity aims to optimise operation containing neutral values.  
Examples:
- `y = x + 0` &#8594; every use of `y` is replaced with `x`
- `a = b * 1` &#8594; every use of `a` is replaced with `b`

#### Strength Reduction
A strength reduction pass replace `mul` instructions with shift instruction to reduce computational complexity. 
Example:
- `x * 15` &#8594; `(x << 4) - x`
- `x * 17` &#8594; `(x << 5) - 15x`
- `x / 16` &#8594; `x >> 4`

Observations:  
In case the element "subtracted" can be optimized, the algorithm will optimize further more the operation. In case it is used on divisions, the divisor **must** be an exact multiple of 2.

#### Multi-instruction optimization
Multi instruction optimization operates in cases where, given a SSA register, the same fixed amount is addend and subtracted from it.
Examples:
- `y = x + 2; z = y - 2` &#8594; every use of `z` is replaced with `x`
- `y = x + 2; z = y / 2` &#8594; every use of `z` is replaced with `x`

## Global Optimizations
`DataFlowAnalysis` folder contains global optimizations algorithms.  
Optimization tasks addressed:
- Very Busy Expressions
- Dominator Analysis
- Constant Propagation

### Loop Invariant Code Motion (LICM)
Instructions that does not change from one iteration to another can be moved outside the loop in order to be executed only once.

Instructions that meet the following requirements are candidate for code motion:
- are invariant
- dominate their uses
- dominate the exits of the loop or are dead outside the loop

Candidated instruction may be moved outside the loop (just before the loop header, in the so called preheader block) in order to be executed only one time.  
  
`LoopOpts.cpp` and `LoopOpts.h` files contain the Loop Invariant Code Motion pass.  
In order to make the pass work, `src/GlobalOpts/LoopOpts.cpp` file must be moved to the following directory:  
```
$SRC/llvm/lib/Transforms/Utils
```
Add `LoopOpts.cpp` in `$SRC/llvm/lib/Transforms/Utils/CMakeLists.txt`.  
`src/GlobalOpts/LoopOpts.h` file must be moved to the following directory:  
```
$SRC/llvm/include/llvm/Transforms/Utils
```
Replace `SRC/llvm/lib/Passes/PassRegistry.def` with the provided `src/PassRegistry.def`.  
Add the follwing line to `SRC/llvm/lib/Passes/PassBuilder.cpp`:
```
#include "llvm/Transforms/Utils/LoopFusion.h"
```
Where `$SRC` is the source folder of the project.  
To compile the source code:
```
cd $ROOT/BUILD
make opt
make install
```

### Loop Fusion
Given two loops that satisfy the follwing requirements:
- are adjacent
    - there cannot be any statement that execute between them
- have the same number of iterations
- are control flow equivalent
    - given two loop Lj and Lk, Lj before Lk, Lk dominates Lj and Lj post-dominates Lk
- are distance independent or have a non negative dependence distance
    -  a negative distance dependence occurs between Lj and Lk, Lj before Lk, when at iteration m from Lk uses a value that is computed by Lj at a future iteration m+n (where n > 0).

Then they can be fused, i.e. the body of the latter is connected after the body of the former.

`LoopFusion.cpp` and `LoopFusion.h` files contain the Loop Fusion pass.  
In order to make the pass work, `src/GlobalOpts/LoopFusion.cpp` file must be moved to the following directory:  
```
$SRC/llvm/lib/Transforms/Utils
```
Add `LoopFusion.cpp` in `$SRC/llvm/lib/Transforms/Utils/CMakeLists.txt`.  
`src/LoopOpts.h` file must be moved to the following directory:  
```
$SRC/llvm/include/llvm/Transforms/Utils
```
Replace `SRC/llvm/lib/Passes/PassRegistry.def` with the provided `src/PassRegistry.def`.  
Add the follwing line to `SRC/llvm/lib/Passes/PassBuilder.cpp`:
```
#include "llvm/Transforms/Utils/LoopFusion.h"
```
Where `$SRC` is the source folder of the project.  
To compile the source code:
```
cd $ROOT/BUILD
make opt
make install
```

#### Example
The example defined in `Test/loop_fus_ex1_virtualregs.ll` shows the loop fusion pass in action.

![loop_before_fusion](/imgs/loop_before_fusion.png)

![loop_after_fusion](/imgs/loop_after_fusion.png)

## Testing
`Tests` folder contains different LLVM files to test the optimizations.

Compile the source code:
```
cd $ROOT/BUILD
make opt
make install
```

To create a file that uses virtual registries starting from the file written in C:
```
opt -p mem2reg <file_name>.c -o <file_name_virtualmem>.bc
llvm-dis <file_name_virtualmem>.bc -o <file_name_virtualmem>.ll
```
This step is needed only for global passes.  
To directly start from the .ll file with virtual registers, both source files (.c and .ll) are provided in the `Tests/` directory.

To run tests:
```
opt -p <optimization_pass_name> <file_name>.ll -o <file_name_optimized>.bc
llvm-dis <file_name_optimized>.bc -o <file_name_optimized>.ll
```

Note:
Implemented passes names are `localopts`, `loopopts` and `loopfusion`.

## Authors
- Raffaele Tranfaglia
- Matteo Venturi
- Mirco Piccinini
