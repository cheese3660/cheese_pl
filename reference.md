# Cheese Language Reference
## Description and Philosophy
The Cheese Programming Language is a language primarily designed by me (Lexi Allen), the main driving goal behind it is to create a language that I would want to use.
But, don't let that stop you from using it, as I am also designing this to be a language for others to use as well.
To this effect, the following few points are somewhat guiding me. (Subject to change as this project goes on)
- Make a compiled language I would want to use
- Make it fun and easy to write
- Also make it easy to understand what is going on, but not to the point where it makes it overly verbose (i.e. allow for hidden control flow with operators and such)
- Don't rely on one Paradigm (i.e. OOP, functional programming, imperative), instead allow for users to use one of their choice, or a mix.
- Don't design for one target only (i.e. only systems, only game dev, etc..)
- Have a decent standard library to make things easier for the programmer, for both bare-metal and hosted
- Have an extremely powerful metaprogramming interface that is readable (unlike C++ TMP)
- Don't be afraid about borrowing ideas from other languages
- Rely on separators as little as possible

A few other goals that are driving this language are: to be able to use this in an OS at some point, and for this language to be able to compile itself

## Standard Library
TODO: Standard library documentation (and standard library)

## Example Program
```cheese
fn fibbo n: i32 => i32
    ==> if n == 0 or n == 1
            1
        else
            fibbo(n-1)+fibbo(n-2)
fn main => i32 entry ==> fibbo(10)
```
Let's go through this example line by line to see what's going on
```cheese
fn fibbo n: i32 => i32
```
This line is declaring a function for us (rather a function template as all function declarations are templates),
`fn` is a token that signifies the start of a function template declaration, it is followed by the name of the function `fibbo`.
Then after the name of the function follows its list of arguments, in this example there is one argument, `n`.
Each argument is followed by a `:` and then the type, which in this case is `i32` or a signed integer with a width of 32 bits.
After the argument list, follows `=>` which signifies that following the token is the return type, which in this case is again `i32`.
```cheese
    ==> if n == 0 or n == 1
```
`==>` is the return statement signifying the expression that follows is the value being returned, or if there is nothing
following it on the rest of the line, it is treated as returning a void value.
`if` starts an if expressions, conditional expressions in cheese can yield values when they are followed by anything but
an unnamed block. After the if statement follows the condition, `n == 0 || n == 1`, in this case. `or` being the boolean or operator.
```cheese
            1
```
Following the condition of the if expression is the body, which is `1` here. All this means is that if the preceding
condition is true then the if expression yields a 1 value.
```cheese
        else
```
Optionally, following the body of an if expression is `else` and then an expression to be run after that if the condition
was not true.
```cheese
            fibbo(n-1)+fibbo(n-2)
```
This is the expression that was following the prior `else` expression, it's calling the function `fibbo` twice and adding the sum.
Notice that arguments in cheese are passed within parentheses (`()`)

## Comments
```cheese
//This is a single line comment
/*
    This is a block comment
*/
/*
    /*
        Block comments support nesting
    */
*/
```
Cheese has 2 types of comments, single line comments denoted by `//` and block comments opened by `/*` and closed with `*/`
## Values
```cheese
// Structure level (top-level) declarations are lazily evaluated and as such are order independant    
let x const = 5 + y //Integer
let y const = 3 //Integer
fn main => void public {
    // Integers
    let z: i32 = 32_241_321
    // Floats
    let f: f32 = 1_234.5678E-3
    // Complex Numbers
    let c: c32 = f+9.1I
    // Booleans
    let b = true
    let b2 = false
    let b3 = not b
    let b4 = b2 and b3
    let b5 = (not b2) xor b3
    // Optional
    let o: Maybe(u32) = 5
    let o2: Maybe(u32) = none
    // Errors
    let ok: Result(u32) = 5
    let err: Result(u32) = .error
    // Arrays 
    let arr: [4]u32 = .[5 6 7 8]
    let arr2 = u32[5 6 7 8]
    // Types
    // Types are first class values at compile time in Cheese
    let int = i32
    // Empty structures are creaeted with struct
    let empty = struct
    // Tuples are created with struct (...)
    let tuple_type = struct (int int)
    // Structures with named fields are created with struct {...}
    let struct_type = struct {
        x: f32
        y: f32
    }
    // Enums are created with enum {...}
    let enumeration = enum(u2) {
        Empty = 1
        Tuple(int int)
        Structure{
            x: f32
            y: f32
        }
    }
    // Constructing objects
    let empty_value = empty()
    let tuple_value = tuple_type(5, 4)
    let tuple_value_2: tuple_type = .(5, 4)
    let struct_value = struct_type{x: 5.5, y: -1.3}
    let struct_value_2: struct_type = .{x: 5.5, y: -1.3}
    let enum_empty = enumeration.empty
    let enum_tuple: enumeration = .Tuple(5 4)
    let enum_structure: enumeration = .Structure{x: 5.5, y: -1.3}
}
```
### Primitive Types
| Type               | Equivalent In C  | Description                                                              |
|--------------------|------------------|--------------------------------------------------------------------------|
| `bool`             | `bool`           | A type who's value can either be `true` or `false`.                      |
| `i(n)`             | `int(n)_t`       | An N bit signed integer type, 0 < N < 65536                              |
| `u(n)`             | `uint(n)_t`      | An N bit unsigned integer type, 0 < N < 65536                            |
| `isize`            | `intptr_t`       | A signed integer that contains as many bits as a pointer                 |
| `usize`            | `size_t`         | An unsigned integer that contains as many bits as a pointer              |
| `f32`              | `float`          | A single precision floating point number                                 |
| `f64`              | `double`         | A double precision floating point number                                 |
| `c32`              | `float complex`  | A complex number consisting of 2 single precision floating point numbers |
| `c64`              | `double complex` | A complex number consisting of 2 double precision floating point numbers |
| `opaque`           | `void`           | An opaque type                                                           |
| `void`             | (none)           | A type that contains 0 bits                                              |
| `comptime_float`   | (none)           | The type of float literals (only allowed at compiletime)                 |
| `comptime_int`     | (none)           | The type of integer literals (only allowed at compiletime)               |
| `comptime_complex` | (none)           | The type of complex literals (only allowed at compiletime)               |
| `comptime_string`  | (none)           | The type of string literals (only allowed at compiletime)                |
| `noreturn`         | (none)           | The return type of expressions that don't return                         |
| `type`             | (none)           | The type of types                                                        |
### Primitive values

| Name            | Description                     |
|-----------------|---------------------------------|
| `true`, `false` | `bool` values                   |
| `none`          | used to set Maybe(T) to `null`  |

### String literals
```cheese
let str = "This is a string literal"
```
String literals are values of type `comptime_string` which can be coerced at runtime to `<>~u8`, `[?]~u8` or `[]~u8`,
which are a constant slice of `u8`, a pointer to an unknown amount of constant `u8`, or an array of `u8`

### Assignment
The `let` keyword binds a value to an identifier, it is by default immutable
```cheese
let x = 3
```
If the value must be mutable add `mut` after the name
```cheese
let x mut = 3
```
To define an uninitialized variable use `def`
```cheese
def x: comptime_int
```
Before this variable is used, all prior code paths must have initialized this variable
### Destructuring
Cheese also allows you to destructure tuples and structures, including nested ones with `let`
```cheese
let internal = struct (i32 i32 i32)
let external = struct {
    a: internal
    b: i32
    c: internal
}
let value = external{
    a: .(5 3 6)
    b: 7
    c: .(9 0 1)
}
let {
    a: (
        x mut
        y
        z
    )
    b: w
    c: _ // We don't care about the c value in the structure
} = value
```

### Functions
Functions in Cheese as described earlier are defined via the `fn` keyword, functions in Cheese are also by definition function
templates and as such you can have multiple definitions for the same function, functions are also values.
```cheese
//Simple one expression function definition
fn square x: any => any ==> x*x
//Multi line function definition
fn complex x: any => any {
    let square_1 = square(x)
    let square_2 = square(x)
    ==> square_1+square_2
}
```
Notice a few things, first off `any` on the return type, that means the return type of the function is deduced from its
body, secondly, that the single line function declaration is not enclosed in `{}` as the block is optional if the body
is a single expression. Functions can also have modifiers that change how they work, they go after the return type
declaration. Possible modifiers are.

| Modifier   | Description                                                                                                                                                                                                      |
|------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `inline`   | Tells the compiler to insert the function body at all call sites.                                                                                                                                                |
| `extern`   | Tells the compiler to use external linkage for this function. i.e. that the functions name doesn't get mangled and it uses a defined ABI                                                                         |
| `export`   | This function always gets made if it gets imported, and the compiler will export this symbol in the resulting object, implicitly `extern`, and also implicitly `public`                                          |
| `comptime` | This function can only be run at compile time                                                                                                                                                                    |
| `public`   | This function is usable outside of the structure it was defined within                                                                                                                                           |
| `private`  | All functions are `private` by default, this only means that this function can only be used within the structure it was defined in                                                                               |
| `entry`    | Used in executables, this tells the compiler that this is the symbol it should start generating code from, if there are multiple functions defined with entry, then the compiler generates the first one it sees |

An example of a function using one of said modifiers is
```cheese
fn doSomething arg: i32 => i32 export ==> arg+1
```

### Importing Functions From Other Objects
To import a function symbol for your program to use, you define a function as you normally would, but you put `import`
after it instead of a body. These are implicitly `extern`. If said function uses `void*` instead use `*opaque` to define
the type.
```cheese
fn malloc amount: usize => *opaque import //Import the malloc function from the C standard library
```
### Anonymous functions
Anonymous functions are functions that are not bound to a name, they do not capture their scope though, examples of
their usage though is for returning functions. They can refer to compile time values in their scope when creating them.
Anonymous functions can be templates.
```cheese
fn accessor index: comptime_int comptime => fn (any) => any
    ==>
        fn x: any => any
            ==> x[index] 
```

### Closures
```cheese
fn main => void entry {
    let closure = lambda x: any 
}
```

### Function Composition
Functions and function-like objects can be composited together with all the arithmetic operators to produce function-like
objects that when called in turn call both of the base functions and perform the operation on the results, this still
respects `and`/`or` short-circuiting. There is also the special function composition operator `|` which "pipes" the
output of the first function into the second one
```cheese
fn square x: any => any ==> x*x
let fourth_first = square(square) //square(square) --> square*square
let fourth_second = square * square //fourth_second(x) --> square(x)*square(x) --> (x*x)*(x*x)
let fourth_third = square | square //fourth_third(x) --> square(square(x)) --> (x*x)*(x*x)
```

### Function Types
Function types are defined by the syntax, these types are comptime only but can be templates
```
fn(args...) => return_type modifiers...
```

### Function Pointers
Function pointers are defined by the above syntax with a `*` prepended, but can only be made of runtime known types
```cheese
let fn_ptr = *fn(i32)=>i32
```
Functions can generally be coerced into pointers, and also taking the address of the function with `&` will also coerce
them, this is if the function is not templated or has multiple definitions, if that is the case then you use
`funct.$FnPtr(...)` where ... is an example of the arguments you are passing in used. You can also do `$FnPtr(funct,...)`

### Structure Types
### Tuple Types
### Empty Structure
### Enumerations
### Results
## Blocks
Blocks in Cheese are enclosed in `{` and `}` characters. And must be put on a separate line or a context where it does
not immediately follow an expression, as then it would be interpreted as an object constructor. Their type is always
`void`
```cheese
fn main => void entry {
    {
        //This is a block.
    }
}
```
## Named Blocks
Named blocks in Cheese are a block label which is an identifier enclosed in `:(` and `)` followed by a block, their
type is the peer type of all `<==(NAME)` statements in the block with the name referencing the block name

```cheese
:(name) {
    <==(name) 5
}
```
## If Expressions
## Match Expressions
## Loop
Cheese contains the `loop` keyword if you wish to loop something infinitely (until it breaks), its return type is
defined by the peer type of `<==` statements within the loop, if none are present, it is `noreturn`
```cheese
fn main => void entry
    loop
        doSomething()
```
### Yielding from loops
Cheese has a yield symbol `<==` which also functions as the break symbol if followed by `none`
```cheese
fn main => void entry
    let x = loop
        if (hasX())
            <== getX()
```
## While Loops
Cheese also has while loops, which loop until the given condition is false. Note, that if you wish for the body to be a
block, you must put `do` after the condition, or put the block on the next line, otherwise it will be interpreted as an
object constructor
```cheese
fn main => void entry
{
    let y: i32 mut = 15
    let cnt: u32 mut = 0
    let x = while true do {
        y -= 1
        cnt +=1
        if cnt == 3 then <== y
    }
}
```
### While Else
If you have a while loop that loops until a condition, that you also want to return a default value from if it breaks
before it can yield, then you use a `while` ... `else` loop
```cheese
fn main => void entry
{
    let y: i32 mut = 15
    let cnt: u32 mut = 0
    let x = while y > 0
    {
        y -= 1
        cnt +=1
        if cnt == 3 then <== y
    } else cnt //if cnt never gets to 3, then return the count instead
}
```
### Continuing
Continuing to the next loop in cheese is done with the `continue` keyword
## For Loops

### For Transformations
### For Else
For else works in the same way as while else, if the loop finishes without yielding a value, it yields the else value