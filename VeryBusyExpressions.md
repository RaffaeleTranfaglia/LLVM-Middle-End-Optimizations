# Very busy expression

## Framework

| Algorithm parameters | Value |
| --- | --- |
| Domain | set of expressions |
| Direction |Backward: <br> $in[b] = f_b(out[b])$ <br> $out[b] = \wedge in[succ(b)]$|
| Transfer function |$f_b(x) = Gen_b \cup (out_b – Kill_b)$ <br> |
| Meet operation |$\cap$|
| Boundary Condition |$in[exit] = \emptyset$|
| Initial interior points |$in[b] = U$|

- The $Gen_b$ are all the expressions used in a basic block. The $Kill_b$ are all the definitions of variables which name is equal to an operand in the expression.
- We consider only binary expressions for simplicity

## Pseudocode

```
input = Control Flow Graph CFG (Nodes, Edges, Entry, Exit)

// Boundary condition
    in[Exit] = ∅

// Initialization
for each basic block B other than Exit
    in[B] = U

while (changes to any in[] occour)
{
    for each basic block B other than Exit
    {
        out[B] = ∩(in[s]) for all successors s of B
        in[B] = Fb(out[B])
    }
}
```

## example

![image](images/Screenshot%202024-04-25%20alle%2016.28.40.png)

Domain: (a-b), (b-a)

||Gen|Kill|
|---|---|---|
|BB2|||
|BB3|b-a||
|BB4|a-b||
|BB5|b-a||
|BB6||a-b <br> b-a|
|BB7|a-b||
|BB8|||

|| Initialization|
|---|---|
||IN[B]|
|BB2|U|
|BB3|U|
|BB4|U|
|BB5|U|
|BB6|U|
|BB7|U|
|BB8|U|

|| Iteration 1||
|---|---|---|
||IN[B]|OUT[B]|
|BB2|Ø $\cup$ {(b-a) - Ø} = b-a| {(b-a),(a-b)} ∩ (b-a) = b-a|
|BB3|(b-a) $\cup$ {(a-b) - Ø} = {(b-a), (a-b)}|a-b|
|BB4|(a-b) $\cup$ {Ø - Ø} = a-b |Ø|
|BB5|(b-a) $\cup$ {Ø - Ø} = b-a|Ø|
|BB6| Ø $\cup$ {(a-b) - (a-b),(b-a)} = Ø |(a-b)|
|BB7|(a-b) $\cup$ {Ø - Ø} = a-b|Ø|
|BB8| Ø||

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