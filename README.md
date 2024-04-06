# LLVM-Middle-End-Optimizations
Machine independent passes to optimise LLVM intermediate representation.  

## Installation
In order to run the optimization passes, it is necessary having LLVM 17.0.6 source code installed.  
Official repository [here](https://github.com/llvm/llvm-project).  

The following steps assume that LLVM 17.0.6 has been installed.  
Clone the current repository:
```
git clone https://github.com/RaffaeleTranfaglia/LLVM-Middle-End-Optimizations.git
```
`src/LocalOpts.cpp` file must be moved to the following directory:  
```
$SRC/llvm/lib/Transforms/Utils
```
Add `LocalOpts.cpp` in `$SRC/llvm/lib/Transforms/Utils/CMakeLists.txt`.  
`src/LocalOpts.h` file must be moved to the following directory:  
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

## Introduced Optimizations

### Constant Folding
Constant Folding execute at compile time operations involving contant values.  
Examples:
- `x = 4 + 9` &#8594; `x = 13 + 0`
- `x = 6 * 2` &#8594; `x = 12 + 0`

Observation:
The assignment of the computed constant is carryed out using an addition of the given constant with zero.  
Afterwards the introduced `add` will furtherly be optimized by the Algebric Identity optimization.

### Algebraic Identity optimization 
Algebraic Identity aims to optimise operation containing neutral values.  
Examples:
- `y = x + 0` &#8594; every use of `y` is replaced with `x`
- `a = b * 1` &#8594; every use of `a` is replaced with `b`

### Strength Reduction
A strength reduction pass replace `mul` instructions with shift instruction to reduce computational complexity. 
Example:
- `x * 15` &#8594; `(x << 4) - x`
- `x * 17` &#8594; `(x << 5) - 15x`
- `x / 16` &#8594; `x >> 4`

Observations:  
In case the element "subtracted" can be optimized, the algorithm will optimize further more the operation. In case it is used on divisions, the divisor **must** be an exact multiple of 2.

### Multi-instruction optimization
Multi instruction optimization operates in cases where, given a SSA register, the same fixed amount is addend and subtracted from it.
Examples:
- `y = x + 2; z = y - 2` &#8594; every use of `z` is replaced with `x`
- `y = x + 2; z = y / 2` &#8594; every use of `z` is replaced with `x`

## Testing
`Tests` folder contains different LLVM files to test the optimizations.

Compile the source code:
```
cd $ROOT/BUILD
make opt
make install
```

To run tests:
```
opt -p localopts <file_name>.ll -o <file_name_optimized>.bc
llvm-dis <file_name_optimized>.bc -o <file_name_optimized>.ll
```

## Authors
- Raffaele Tranfaglia
- Matteo Venturi
- Mirco Piccinini
