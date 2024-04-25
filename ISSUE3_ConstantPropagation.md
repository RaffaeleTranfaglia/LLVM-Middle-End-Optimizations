# Constant progation

Framework

Domain | all couples (var, constant_value)
--- | ---
Direction | forward <br> out[B] = F<sub>B</sub>(in[B]) <br> in[B] = M(out[predecessorB])
Transfer function (F) | F<sub>B</sub>(in[B]) = Gen<sub>B</sub> U (in[B] - Kill<sub>B</sub>) <br> Gen<sub>B</sub> and Kill<sub>B</sub> are determined by analyzing the instructions of the block B in descending order: the generated couples are the locally available definitions of all the variables where the right hand side is a constant or can be considered constant considering all couples that have been propagated by the specific define instruction; to determine the killed couples it must be considered that any definition of a variable kills all the other couples with the same variable present in the universe set <br> out[inst] = F<sub>inst</sub>(in[inst]) = Gen<sub>inst</sub> U (in[inst] - Kill<sub>inst</sub>) <br> where <br> Gen<sub>inst</sub> = (defined_var, const_val) if the right hand side can be considered constant, which means it only contains constants or variables that are contained in the couples propagated by the preceding instruction or block, <br> in[inst] are the couples propagated by the preceding instruction and in[inst] = in[B] for the first instruction of the block, <br>  Kill<sub>inst</sub> = (defined_var, const), "const" staying for any constant value
Meet operator (M) | intersection
Boundary condition | out[entry] = ∅
Initial interior points | out[B] = ∩


Pseudocode:
```
input = Control Flow Graph CFG (Nodes, Edges, Entry, Exit)

Universe = ∅
for all variables x defined in the entire function, add (x,const) to Universe

// Boundary condition
out[Entry] = ∅

// Initialization
for each basic block B
    out[B] = Universe

while (changes to any out[] occour)
{
    for each basic block B other than Entry
    {
        in[B] = ∩(out[p]) for all predecessors p of B
        out[B] = F_B(in[B])
    }
}
```


Observations:
-  

$$ \text{hello world} $$