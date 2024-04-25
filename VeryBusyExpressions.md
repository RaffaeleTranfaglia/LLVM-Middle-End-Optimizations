# Very busy expression

## Framework

| Algorithm variable | Description |
| --- | --- |
| Domain | sets of expressions |
| Direction |Backward: <br> $in[b] = f_b(out[b])$ <br> $out[b] = \wedge in[succ(b)]$|
| Transfer function |$f_b(x) = Use_b \cup (out_b – Def_b)$ <br> The $Use_b$ are all the expressions used in a basic block. The $Def_b$ are all the definitions of variables which name is equal to an operand in the expression. |
| Meet operation |$\cap$|
| Boundary Condition |$in[entry] = \emptyset$|
| Initial interior points |$in[b] = U$|

## Pseudocode

```
input = Control Flow Graph CFG (Nodes, Edges, Entry, Exit)

// Boundary condition
    in[b] = ∅

// Initialization
for each basic block B
    in[B] = ∅

while (changes to any in[] occour)
{
    for each basic block B other than Exit
    {
        out[B] = u(in[s]) for all successors s of B
        in[B] = Fb(out[B])
    }
}
```

## example

![image](images/Screenshot%202024-04-25%20alle%2016.28.40.png)

||Gen|Kill|
|---|---|---|
|B2|||
|B3|b-a||
|B4|a-b||
|B5|b-a||
|B6||a-b <br> b-a|
|B7|a-b||
|B8|||

|| Initialization||
|---|---|---|
||IN[B]|OUT[B]|
|BB2|$\emptyset$||
|BB3|$\emptyset$||
|BB4|$\emptyset$||
|BB5|$\emptyset$||
|BB6|$\emptyset$||
|BB7|$\emptyset$||
|BB8| $\emptyset$||

|| Iteration 1||
|---|---|---|
||IN[B]|OUT[B]|
|BB2|$\emptyset$ $\cup$ {{b-a} - $\emptyset$} = b-a|{b-a,a-b} meet_operator {b-a} = b-a|
|BB3|{b-a} $\cup$ {{a-b} - $\emptyset$} = {b-a, a-b}|a-b|
|BB4|{a-b} $\cup$ {$\emptyset$ - $\emptyset$} = a-b |$\emptyset$|
|BB5|{b-a} $\cup$ {$\emptyset$ - $\emptyset$} = b-a|$\emptyset$|
|BB6|$\emptyset$ $\cup$ {(a-b) - (a-b)} = $\emptyset$ |a-b|
|BB7|{a-b} $\cup$ {$\emptyset$ - $\emptyset$} = a-b|in[BB8] = $\emptyset$|
|BB8| $\emptyset$||

Changes have been registered

|| Iteration 2||
|---|---|---|
||IN[B]|OUT[B]|
|BB2|b-a|b-a|
|BB3|{b-a, a-b}|a-b|
|BB4|a-b |$\emptyset$|
|BB5|b-a|$\emptyset$|
|BB6|$\emptyset$ |a-b|
|BB7|a-b|$\emptyset$|
|BB8| $\emptyset$||

No changes registered