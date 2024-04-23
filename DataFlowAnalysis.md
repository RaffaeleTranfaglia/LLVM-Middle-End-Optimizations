# Data Flow Analysis

General pseudocode for forward data flow:
```
input = Control Flow Graph CFG (Nodes, Edges, Entry, Exit)

// Boundary condition out[Entry]

// Initialization
for each basic block B
    out[B] = neutral set based on meet operator

while (changes to any out[] occour)
{
    for each basic block B other than Entry
    {
        in[B] = meet_operator(out[p]) for all predecessors p of B
        out[B] = TransferFunction(in[B])
    }
}
```

General pseudocode for backward data flow:
```
input = Control Flow Graph CFG (Nodes, Edges, Entry, Exit)

// Boundary condition in[Exit]

// Initialization
for each basic block B
    in[B] = neutral set based on meet operator

while (changes to any in[] occour)
{
    for each basic block B other than Exit
    {
        out[B] = meet_operator(in[s]) for all successors s of B
        in[B] = TransferFunction(out[B])
    }
}
```

Observations:
- The meet operator is defined basing on the nature of the analysis.

- The default domain's state initialization (bit vector) at the end/begin of each basic block depends on the meet operator.

- Boundary condition is the conservative hypothesis of the data initial state. 

- Transfer Function scope is to convey information regarding data's domain through the current basic block. Instructions visit order depends on the direction of the flow analysis. 

$$ \text{hello world} $$