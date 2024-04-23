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
for each basic block B other than Exit
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

$$ \text{hello world} $$