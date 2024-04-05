# LLVM-Middle-End-Optimizations
Machine independent passes to optimise LLVM intermediate representation.  

## Installation
In order to run the optimization passes, it is necessary having LLVM 17.0.6 source code installed.
Official repository [here](https://github.com/llvm/llvm-project).  

Clone the current repository:
```
https://github.com/RaffaeleTranfaglia/LLVM-Middle-End-Optimizations.git
```
LocalOpts file must be moved into the following directory to work properly:  
```
cd LocalOpts.cpp $SRC/llvm/lib/Transforms/Utils
```
Where `$SRC` is the source folder of the project.  
To compile the source code:
```
cd $ROOT/BUILD
make -j4
```
  
Otherwise, to install the source code already containig the optimized passes: [here](https://github.com/Glixes/LLVM_middle_end).

## Introduced Optimizations

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

## Authors
- Raffaele Tranfaglia
- Matteo Venturi
- Mirco Piccinini
