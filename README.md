# Dodecamorph

Dodecamorph is an esoteric programming language inspired by brainfuck, befunge and others. I got the idea while watching this [video on esolangs](https://www.youtube.com/watch?v=cQ7bcCrJMHc).

Here's the spec of the language, TLDR data is laid out as N-dimensional, lazily growing arrays, instructions are data, there are 12 instructions and 2 special values

```
Starting conditions:

- The programmer writes instructions into a 1D or 2D array of cells. Each cell is a number or parens delimited by white space (tab or space) on each side, and each row is delimited by a carriage return. At the start of the program, this 1D or 2D array is the instruction tensor.
- Any row starting with / is ignored
- At start of the program, tensor (0) is the instruction tensor and tensor (1) is the data tensor, the data and instruction cursor values are both 0 and the instruction cursor moves by 1 in dimension 0

Instructions:

- instruction 0 outputs the current data cell's contents
- instruction 1 increments the data cursor's cell index by the numbers specified in ()
- instruction 2 sets the data cursor's tensor index to the index specified in ()
- instruction 3 increments the current data cell by 1
- instruction 4 decrements the current data cell by 1
- instruction 5 sets the direction of the instruction cursor to the direction specified in (), where 0 denotes neutral movement, 1 incremental movement and 2 decremental movement
- instruction 6 queries the user for a number and sets the current data cell to it
- instruction 7 sets the instruction cursor's cell index to index specified in () if the current data cell is 0, and then executes the current instruction
- instruction 8 sets the current data cell to opening parens.
- instruction 9 sets the current data cell to closing parens.
- instruction 10 sets the instruction cursor's tensor index to the index specified in ()
- instruction 11 shrinks the tensor at the index specified in () back down to 1 dimension and sets its only remaining element to 0

Instruction rules: 

- Instruction numbers are wrapped by 12
- If an instruction expects () to be provided and none are found in the direction of the instruction cursor, empty parens are implied

Parens rules:

- When specifying (), the parens immediately to the direction of the instruction cursor are used. Each cell corresponds to one dimension. If there are not enough cells between the two parentheses for the dimensions of the affected object, the rest are implicitly 0.
- When an opening parens is found, parens pairing is used to determine its closing parens. The current direction of the instruction cursor is used to traverse the instruction tensor, starting at the instruction cursor. Found opening parens add 1 to the parens count and found closing parens remove 1, until the count is 0. The resulting dimensions are all numbers for which the count was 1.
- If an opening or closing parens is incremented or decremented, the resulting value is 0
- When executed, opening parens move the instruction cursor to their closing parens.
- When executed, closing parens do nothing.

Ticking rules:

- Each tick, the current instruction is executed and the instruction cursor moves in the instruction cursor direction.
- Each element of a direction is wrapped by the number of directions, 3

Tensor rules:

- An index into a tensor is wrapped by the tensor's dimensions.
- A tensor expands if an instruction it is affected by specifies more dimensions than it currently has
- When a tensor expands, new cells are 0
- If the tensor affected by an instruction does not exist, a new tensor is created with the dimensions of the affected cursor. Any tailing dimensions with value 0 are ignored for this purpose, e.g. if the cursor is at 1 2 0 1 0 0 the dimensions of the new tensor is 4

Termination conditions:

- If the instruction cursor does not move during a tick, the program terminates.
- If during parens pairing the original opening parens is found again, the program terminates.

Nomenclature:

- "Cell" denotes an element of a tensor which may contain a number or a parens.
- "Cursor" denotes a tensor index and a cell index.
- "Tensor" denotes an n-dimensional, growable array of cells.
- "Current instruction" denotes the instruction cell the instruction cursor's cell index points to. 
- "Current data cell" The data cell the data cursor's cell index points to.
- "Instruction tensor" denotes the tensor the instruction cursor's tensor index points to.
- "Data tensor" denotes the tensor the data cursor's tensor index points to.
- "Object" denotes a tensor, a cursor, a cell, a tensor index, a cell index or a direction.
- "Affected" denotes an object which an instruction specifies the value of in ()

Miscellaneous:
- Numbers are signed integers from -2147483648 to 2147483647
```

The source code in this repo is a probably insanely buggy interpreter for dodecamorph. Use at your own risk! Also, the spec probably has holes/tacit assumptions, do feel free to point them out!
