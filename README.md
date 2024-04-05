# LLVM-Middle-End-Optimizations
Machine independent passes to optimise LLVM intermediate representation.  

## Installation
In order to run the optimization passes, it is necessary having LLVM 17.0.6 source code installed.
Official repository [here](https://github.com/llvm/llvm-project).  

Clone the current repository:
```
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
make opt
```

To install the source files already containig the optimized passes: [here](https://github.com/Glixes/LLVM_middle_end).

## Optimizations

### Algebraic Identity optimization 
Algebraic Identity is an optimization made on Instruction where it's possible simplify expression with algebraic rules. I can apply Algebraic Identity on expressions like `y = x + 0` or `a = b * 1` ... to obtain `y = x` or `a = b`

In our project we have replaced the IR form all uses of y with x. This operation is possible cause we unlinked the instruction register of y from the CFG, replacing all his uses with *x*; the unlinked instruction will be marked as "dead code" and will be eliminated in Dead Code elimination

### Strength Reduction

A strength reduction operation replace mul instructions with shift instruction to reduce computational complexity of simple operations. For example I can replace the expression `x * 15` with `(x << 4) - x`, replacing a mul with a shift and a subtraction.  

To complete this task we extracted the ConstantInt in the instruction and used the APInt method `ceilLogBase2` which computes the $log_2$ of the number. So for example $ceilLogBase2(17) \rightarrow log_2(32) \rightarrow 5$. After that operation we subtract the "rest" multiplied for x. For example: 

$$x \times 17 \rightarrow (x << 5) - 15x$$

In case the element "subtracted" can be optimized, the algorithm will optimize further more the operation. This operatation can be used even for subdivision, but in this case the divisor **must** be a multiple of 2

### Multi-instruction optimizations

Multi instruction optimization is an operation made on expressions like `y = x + 2; z = y -2` which can be optimized in this way: `y = x + 2; z = x`. 

This optimization has been done using uses of Instructions. For every uses, we substitute the symbol representing the instruction with the instruction itself. 

We implemented that searching for uses of `y`, checking future instructions. Once checked, we replace uses of `z` with `x`.
