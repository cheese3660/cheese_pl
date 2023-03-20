---
title: 'Cheese Language Reference'
subtitle: 'A Reference of The Cheese Programming Language, Including Syntax, Builtins, and The Standard Library.'
author: 'Lexi Allen'
date: 'September 2022'
monofont: "Fira Code"
---

\pagebreak
\tableofcontents
\pagebreak

# Cheese Language Reference

## Description and Philosophy

The Cheese Programming Language is a language primarily designed by me (Lexi Allen), the main driving goal behind it is
to create a language that I would want to use.
But, don't let that stop you from using it, as I am also designing this to be a language for others to use as well.
To this effect, the following few points are somewhat guiding me. (Subject to change as this project goes on)

* Make a compiled language I would want to use
* Make it fun and easy to write
* Also make it easy to understand what is going on, but not to the point where it makes it overly verbose (i.e. allow
  for hidden control flow with operators and such)
* Don't rely on one Paradigm (i.e. OOP, functional programming, imperative), instead allow for users to use one of their
  choice, or a mix.
* Don't design for one target only (i.e. only systems, only game dev, etc..)
* Have a decent standard library to make things easier for the programmer, for both bare-metal and hosted
* Have an extremely powerful metaprogramming interface that is readable (unlike C++ TMP)
* Don't be afraid about borrowing ideas from other languages
* Rely on separators as little as possible

A few other goals that are driving this language are: to be able to use this in an OS at some point, and for this
language to be able to compile itself

------------------------------------------------------------------------------------------------------------------------

# Example Program

```cheese
fn fibbo n: i32 => i32
    if n == 0 or n == 1
        1
    else
        fibbo(n-1)+fibbo(n-2)
fn main => i32 entry fibbo(10)
```

Let's go through this example line by line to see what's going on

```cheese
fn fibbo n: i32 => i32
```

This line is declaring a function for us (rather a function template as all function declarations are templates),
`fn` is a token that signifies the start of a function template declaration, it is followed by the name of the
function `fibbo`.
Then after the name of the function follows its list of arguments, in this example there is one argument, `n`.
Each argument is followed by a `:` and then the type, which in this case is `i32` or a signed integer with a width of 32
bits.
After the argument list, follows `=>` which signifies that following the token is the return type, which in this case is
again `i32`.

```cheese
    if n == 0 or n == 1
```

Since this function is only a single expression, no return statement is necessary. `if` starts an if expressions,
conditional expressions in Cheese can yield values when they are followed by anything but an unnamed block. After the if
statement follows the condition, `n == 0 or n == 1`, in this case. `or` being the boolean or operator.

```cheese
            1
```

Following the condition of the if expression is the body, which is `1` here. All this means is that if the preceding
condition is true then the if expression yields a 1 value.

```cheese
        else
```

Optionally, following the body of an if expression is `else` and then an expression to be run after that if the
condition
was not true.

```cheese
            fibbo(n-1)+fibbo(n-2)
```

This is the expression that was following the prior `else` expression, it's calling the function `fibbo` twice and
adding the sum.
Notice that arguments in Cheese are passed within parentheses (`()`)

```cheese
fn main => i32 entry fibbo(10)
```

This is declaring the main function as the entrypoint, and that it returns an `i32` that is the result of calling
`fibbo` with 10 as `n`.

------------------------------------------------------------------------------------------------------------------------

# Syntax

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

Cheese has 2 types of comments, single line comments denoted by `//` and block comments opened by `/*` and closed
with `*/`

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
    let o: Optional(u32) = 5
    let o2: Optional(u32) = none
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

| Name            | Description            |
|-----------------|------------------------|
| `true`, `false` | `bool` values          |
| `none`          | Literal of type `void` |

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

### Structure Types

Structure types in Cheese are defined with the keyword `struct` followed by `{` or optionally an `impl` keyword plus a
list of interfaces this structure implements. And then within the structure definition is a list of fields within the
structure, plus structure level declarations such as functions or constants or structure level variables.

Note: any file in Cheese is by default a structure and can have all these declarations as well.

```cheese
let coordinate = struct {
    x: i32
    y: i32
    
    fn create x: i32, y: i32 => Self .{x: x, y: y}
    let z = 0
}
```

Note a few things, fields are defined by an identifier followed by a `:`, and then a type. Structures can also have
"static" variables with the `let` or def keywords, and that functions can be declared within structures.

Another thing of note is that all files implicitly declare a structure containing the files contents. So a file can have
field declarations and such.

#### Structure Literals

To create an object out of a structure type, you use the structure name with the `{}` followed by the values inside the
structure or if the structure type is implicitly known you use the `.{}` structure creation operator.

```cheese
let coordinate1 = coordinate{x: x, y: y}
let coordinate2: coordinate = .{x: x, y: y}
```

### Tuple Types

Tuple types are defined in Cheese with the keyword `struct` followed by parentheses.

```cheese
let coordinate = struct (
    i32
    i32
)
```

Tuples cannot have member functions, nor can they implement interfaces.

#### Tuple Literals

To create an object out of a tuple type, you use the tuple name with the `()` followed by the values inside the tuple or
if the tuple type is implicitly known you use the `.()` tuple creation operator.

```cheese
let coordinate1 = coordinate(5, 3)
let coordinate2: coordinate = .(5, 3)
```

### Empty Structure

An empty structure type can be defined in Cheese by the keyword `struct` followed by nothing

```cheese
let Empty = struct
```

### Enumerations

Enumerations in Cheese are defined via the `enum` keyword. They are then followed by a list of enumerated items. These
items can be followed by `{...}` or `(...)` much like structure and tuple declarations. But they may not have member
functions. After the enumerated item then
is optionally followed by an `=` and a value of the item. Optionally following the `enum` keyword is an integer type
which is the "containing" type of the enumeration

```cheese
let Node = enum u8 {
    Empty = 0
    Integer(i32)
    BinaryNode {
        lhs: *~Node
        rhs: *~Node
    }
}
```

#### Enum Literals

Values of enumerations can then be declared with the field name preceded by a `.` if the type is implicitly known, or
the full enumeration name followed by a `.` followed by the field name to explicitly declare the enum type. if it is
a structural or tuple field, then following this is either a `{...}` or `(...)`.

```cheese
let node1 = Node.Empty
let node2: Node = .Integer(5)
let node3: Node = .BinaryNode{lhs: node1, rhs: node2} //Implicitly taking a reference here
```

### References

Reference types in Cheese are defined via the `*`/`*~` operators prefixed in front of a type, with the latter meaning a
reference to an immutable object. When initializing a reference for the first time it takes the address of the argument,
but from then on uses the `operator =` overload to initialize the reference (if it is a mutable reference and the
operator is defined), or it if using the `:=` (Reference reassignment) it re-assigns the reference to refer to the new
value. References can implicitly be taken using the `&` operator, if the value being referred to is constant then that
operator takes an immutable reference, otherwise it takes a mutable reference.

References except for the `:=` operation behave exactly the same as the original objects. There are special builtins for
say doing reference math and such.

```cheese
let x: i32 mut = 5
let y: i32 = 6
let _x = &x
_x = y // This changes the value located at x to point to y
_x := y // This changes _x to point to y
```

### Arrays

Array types in Cheese are defined via the `[]x`/`[]~` operators prefixed before a type, within the `[]` you specify a
list of dimension sizes with `_` meaning an inferred dimension size. Arrays can have any number of dimensions.

```cheese
let vec = [5]f32
let mat = [5, 5]f32
let z: [_]f32 = .[1.2345,6.7890] // Infers a size of 2
```

#### Array Literals

Array literals in Cheese are either created by a type name with `[items...]` following after them or a `.[items...]` to
infer the type name. To create multidimensional literals you have inside the array literal a `.[subitems...]`

```cheese
let x: mat = .[
    .[1,2,3,4,5]
    .[1,2,3,4,5]
    .[1,2,3,4,5]
    .[1,2,3,4,5]
    .[1,2,3,4,5]
]

let y = u32[1,2,3,4,5]
let z = u32[
    .[1,2]
    .[3,4]
    .[5,6]
]
```

#### Subscripting Arrays

To subscript an array you use the `[]` operator, and provide a number of arguments that is strictly less than or equal
to the amount of dimensions of the array, if you provide a number of arguments less than the amount of dimensions, then
it will return an array that is N dimensions less that.

```cheese
let x0 = x[0] //returns a [5]f32 reference
let x1 = x[0,1] //returns an f32
let x2 = x0[1] //also returns an f32
```

### Arrays to an Unknown Amount of Items

To create a type that refers to an array of an unknown amount of items, you use `?` as the dimension and must have this
`?` as the final dimension in the array, creating literals for this is the same, but you must either explicitly cast the
array to an unknown array, or have it implicitly known.

```cheese
let unknown_type = [?]u32
let unknown_value_1 = (.[1, 2]) @ unknown_type
let unknown_value_2: unknown_type = .[1, 2]
```

### Slices

Slices are pointers to a known amount of items, they are created using either `<>` or `<>~` in front of a type, and are
singly dimensional.

```cheese
let slice_type = <>u32
```

#### Slice Literals

Slice literals are the same as array literals, but you must explicitly either cast the array literal to a slice, or have
it implicitly known that it is a slice

```cheese
let slice_value_1 = (.[1, 2]) @ slice_type
let slice_value_2: slice_type = .[1, 2]
```

### Results

Results in Cheese are created via the `Result`, `Result(T)`, and `Result(T,E)` builtins, by default `T` and `E`
are `void`.

```cheese
let res = Result(u32)
```

#### Creating Result Literals

If possible values implicitly cast into `.Ok(T)` or `.Error(E)` if they either of type `T` or of type `E` and returning/
yielding nothing back to something that expects a result will automatically cast into a `.Error` if `T` is not `void` or
optional. But to explicitly create them you can use the `Ok(T)` and `Error(E)` enumeration constants.

```cheese
==> .Error(5)
```

#### Try

Cheese provides a builtin `try` method/macro on error values that if the value is an error, it returns the error (and
does implicit casting of the `E` value if the `E` value of the function is different from the `E` value of the method,
but it still keeps the stack trace).

```cheese
let value = functionThatReturnsError().try
```

#### Error Tracing With Results

Everytime an error result is returned, it keeps track of the address of the function that called it, and more
specifically the address at the point of it being returned. This can be used to provide an exact error trace with
results, provided debug symbols are accessible. And the standard library entry function already does this if the `main`
function returns a result.

i.e. it keeps a stack trace slice, which can be accessible via `$getErrorStackTrace(E)`

To facilitate this Cheese, when it determines a function will return a `Result`, it will add an invisible stack tracing
parameter, and at the bottom of the call stack, where the first non `Result` returning function is, it will generate a
stack trace object. This object can store up to 256 addresses deep (change-able by the build system, up to 65536). (TODO
at some point add call-graph analysis to determine how big this trace should be, like Zig)

### Optionals

Optionals in Cheese are defined via the `Optional(T)` type, this is internally an enum with 2 fields `.None` and
`.Some(T)`

#### Optional literals

Values of type `T` implicitly convert to an `.Some(T)` when using optionals, and values of type `void` implicitly
convert to `.None` unless it is an `Optional(Void)` or an `Optional(Optional(T))`, though you can create both things
with the enum literals.

### Functions

Functions in Cheese as described earlier are defined via the `fn` keyword, functions in Cheese are also by definition
function
templates and as such you can have multiple definitions for the same function, functions are also values.

```cheese
//Simple one expression function definition
fn square x: any => any x*x
//Multi line function definition
fn complex x: any => any {
    let square_1 = square(x)
    let square_2 = square(x)
    ==> square_1+square_2
}
```

Notice a few things, first off `any` on the return type, that means the return type of the function is deduced from its
body, secondly, that the single line function declaration is not enclosed in `{}` as the block is optional if the body
is a single expression, and in that case no return statement is necessary. Functions can also have modifiers that change
how they work, they go after the return type declaration. Possible modifiers are as follows

<!---                  V Note the padding must be to here to prevent overlap in the pdf!--->

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
fn doSomething arg: i32 => i32 export arg+1
```

#### Returning Values From Functions

Values are implicitly returned from one line functions with no return statements. But otherwise to return a value from a
function you use the `==> value` or `==>` keywords, where the latter is the same as returning nothing from the function.
(note to do so in a one line environment you must use the keyword none after it like such `==> none`)

#### Importing Functions From Other Objects

To import a function symbol for your program to use, you define a function as you normally would, but you put `import`
after it instead of a body. These are implicitly `extern`. If said function uses `void*` instead use `*opaque` to define
the type.

```cheese
fn malloc amount: usize => *opaque import //Import the malloc function from the C standard library
```

#### Anonymous functions

Anonymous functions are functions that are not bound to a name, they do not capture their scope though, examples of
their usage though is for returning functions. They can refer to compile time values in their scope when creating them.
Anonymous functions can be templates.

```cheese
fn accessor index: comptime_int comptime => fn (any) => any
    fn x: any => any
        x[index]
```

#### Closures

Closures in Cheese start with a list of arguments enclosed in `|` tokens. following this list of arguments is an
option list of captures enclosed in `[]`, by default if no list is supplied it defaults to `[*~]` or implicit constant
reference capture.

| Capture Syntax | Meaning                                     |
|----------------|---------------------------------------------|
| `*`            | Implicitly capture by mutable reference     |
| `*~`           | Implicitly capture by immutable reference   |
| `=`            | Implicitly capture by value                 |
| `*x`           | Explicitly capture x by mutable reference   |
| `*~x`          | Explicitly capture x by immutable reference |
| `=x`           | Explicitly capture x by value               |

Following the optional list of captures is an optional `=>` followed by a return type,
if none is supplied it defaults to figuring out the return type implicitly. After this is the body, if the body is an
expression that is not enclosed in `{}` then no return statement is needed it just returns the evaluation of the body,
otherwise if the body is a block, a return statement is necessary.

```cheese
fn main => void entry {
    let closure = |x: i32| 5*x
    let closure2 = |x: i32|[*~closure] => i32 closure(x)
}
```

#### Function Composition

Functions and function-like objects can be composited together with all the arithmetic operators to produce
function-like
objects that when called in turn call both of the base functions and perform the operation on the results, this still
respects `and`/`or` short-circuiting. There is also the special function composition operator `|` which "pipes" the
output of the first function into the second one

```cheese
fn square x: any => any ==> x*x
let fourth_first = square(square) //square(square) --> square*square
let fourth_second = square * square //fourth_second(x) --> square(x)*square(x) --> (x*x)*(x*x)
let fourth_third = square | square //fourth_third(x) --> square(square(x)) --> (x*x)*(x*x)
let fourth_fourth = (|x: f64| x*x)*(|x: f64| x*x)

```

#### Function Types

Function types are defined by the syntax, these types are comptime only but can be templates

```
fn(args...) => return_type modifiers...
```

#### Function Pointers

Function pointers are defined by the above syntax with a `*` prepended, but can only be made of runtime known types

```cheese
let fn_ptr = *fn(i32)=>i32
```

Functions can generally be coerced into pointers, and also taking the address of the function with `&` will also coerce
them, this is if the function is not templated or has multiple definitions, if that is the case then you use
`funct.$FnPtr(...)` where ... is an example of the arguments you are passing in used. You can also
do `$FnPtr(funct,...)`

### Generators

Generators in Cheese act like a function that can be called multiple times with no parameters that returns an
`Optional(T)`, and once they exit without returning a `T` they return `.None` every single call afterwards. Generators
in Cheese follow the same syntax as functions/anonymous functions/function pointers but the parameters set up the
initial state of the generator. The `Optional` part of the return type is not specified, just the `T`

```cheese
generator numbersUpTo n: i32 => i32
{
    let current: i32 mut = 0
    while current <= n
    {
        ==> current
        current+=1
    }
    // Implicit return .None, so it exits here
}
// Note, values of type generator must be declared mutable
// as the internal state of a generator changes.
let x mut = numbersUpTo(32)
x() //0
x() //1
x() //2
x() //3
```

### Interfaces

Interfaces are defined via the `interface` keyword followed by brackets.
Inside the brackets then follows a list of function declarations, the functions cannot be extern, comptime, export,
etc..., and all the types must be concretely defined and not inferred. You can still define operators from within
interfaces.

```cheese
let Nameable = interface {
    fn setName self, name: std.String => void
    fn getName ~self => std.String //~self means that this function will not modify internal state
}
```

Interfaces can be combined with the `&` keyword

```cheese
let C = A & B
```

#### Implementing Interfaces

Interfaces can be implemented for structures via the `impl` keyword following the structure.

```cheese
let Person = struct impl Nameable {
    ...
    fn setName self, name: std.String => void pub
        self.name = name
    fn getName ~self => std.String pub
        ==> self.name
}
```

For functions that you are implementing in an interface, they must have a "self" parameter which is either an immutable
reference or mutable reference to an object of the type of the structure implementing said interface, depending on
whether the interfaces function was defined immutable or not.

Interfaces can also be implemented as a kind of mix-in sort of thing using the following syntax

```cheese
impl SomeType: interfaces... {
    ...
}
```

The body of the implementation is the same as a structure, it is just unable to contain any fields

Interfaces finally can be implemented for a generic type using a function call kind of syntax

```cheese
impl SomeType(A: type, B: type): interfaces... {
    ...
}
```

This is a strange thing on how it works, it looks for a function that returns a type called SomeType and with the
arguments
provide and attaches itself to all the instances of that returned function that have already been created and to any new
ones when it is called. Constraints can be used for the types to make it attach to specific forms of the generic type
only.

Note: Internally interfaces are implemented as essentially the following

```cheese
let _Nameable = struct {
    let _VTable = struct {
        setName: *fn *opaque, std.String => void
        getName: *fn *~opaque => std.String
    }
    _obj: *opaque //Maybe *~ for a const interface reference
    _vtable: *_VTable
    fn setName ~self, name: std.String => void pub inline
        self._vtable.setName(self._obj,name)
    fn getName ~self => std.String pub inline
        self._vtable.getName(self._obj)
    // plus the following if the runtime type information is enabled by the build system
    _ty: usize //maybe just u32 but ehh
    fn _is ~self, ty: type => bool pub inline
      ty.__type__ == self._ty
    fn _convert: ~self, ty: type => Optional(*ty) pub inline
      if self._is(ty) then _obj else .None
}
```

#### Extending Interfaces

Interfaces can imply implementation of other interfaces by using the `impl` keyword in the interface implementation.
This mandates that any object implementing this interface must also implement the interfaces it "implements", it also
means that any interface can be converted up the tree of interfaces, but not down

```cheese
let A = interface ...
let B = interface impl A ...
```

#### Utilizing Interfaces

```cheese
fn nameObject input: *Nameable => void
    input.setName(generateRandomName())
fn getObjectName input: *~Nameable => std.string
    input.getName()
```

Note how to refer to an interface type at runtime you must use a reference or const reference, this is for 2 reasons,
it makes it more obvious that the interface contains a reference to an object rather than a copy of an object, and also
it maintains mutability

#### Enabling RTTI For an Interface

RTTI (Runtime Type Information) can be enabled for an interface by putting the `dynamic` keyword after the `interface`
keyword. This allows for dynamic type checking and casting of interface values. Note, by enabling this, it adds an extra
field to the interface values that will increase the size of the interface value by a factor of 50%.

#### RTTI Casting

If RTTI is enabled, then the `is` keyword is enabled, and so is the `@*` cast operator, with these keywords you can get
if an interface is a type down the implementation tree, and/or the original type of the interface via `is`, and safely
dynamically cast back to the original type with the `@*` operator which returns an `Optional(*T)`

### Concepts

Concepts are defined via the `concept` keyword followed by brackets.

```Cheese
let Number = concept {
    _ = value + value
    _ = value * value
    _ = value / value
    _ = value % value
    _ = value - value
    _ = value > value
    _ = value < value
    _ = value == value
    _ = value >= value
    _ = value <= value
    _ = value != value
    $conceptAssert(!Type.$IsFn()) //conceptAssert is an assert for inside a concept
}
//Concepts can be used in place of Types, but also can be called on a type, and it will return a bool
let x: Number = 5 @ i32 // The type of x is deduced to be i32
let y = Number(void) // false
```

Concepts can be combined much like interfaces with the `&` keyword

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

If expressions in Cheese are begun with the `if` keyword, and then followed by a condition, and optionally the `then`
keyword, followed by the body of the if expression, and finally, optionally an `else` following the body of the
expression.

If expression's in Cheese are called "expressions" as they can return values, if the body of the if statement is not an
unnamed block and nor is the else statement, then the returned value from the statement is the value of the 2 blocks
converted to the peer type between them.

```cheese
fn abs x: i32 => i32 if x == 0 then -x else x
```

### If Unwrapping

If the condition in an if statement is an `Optional(T)` and following the condition is a `:` followed by an identifier
or an identifier with a `*` or `*~` in front of it, then the if statement is used to unwrap the optional and then assign
the value within the optional to the name

```cheese
if SomeInterface @* SomeType : someTypeValue
  ...
```

## Match Expressions

Match expressions in Cheese are begun with the `match` keyword, then the value to match to, then `with` if the brackets
are on the same line as the `match` otherwise a newline, then `{`, they are then followed by a list of "arms" which
follow
the match arm syntax. Then closed by `}`.

### Match Arms

Match arms are a list of possible constraints followed by the `=>` symbol, for anything with destructuring, only one
constraint can exist per arm. Any list of constraints can also be followed by the `-> name` syntactical structure to
store the result of the constraint into said variable. This is especially useful in nested constraints especially inside
tuple destructuring.

#### Value Constraint

Value constraints are just that, values, such as `3` or `.None`, or compile time expressions that result in a value

```cheese
3, 6, 5 => ...
```

#### Range Constraint

Range constraints are a value constraint followed by `..` followed by another value constraint.
Range constraints are inclusive of both ends

```cheese
1..10 => ....
```

#### Function Constraint

Functional constraints are defined by the `constrain` keyword followed by a functional object that takes in the type of
the switched value, and returns a bool.

```cheese
constrain isEven => ...
```

#### Catchall Constraint

The catchall constraint is `_`. This matches anything that hasn't already been matched by
other arms.

#### Destructuring Constraint

The destructuring constraint for structures is a `.{` followed by a list of field variable names with constraints, and
by default, if no variable name is provided with the `->` syntax then the field name is used, unless the constraint is
`_`. If only the field name is provided and no `: constraint...` then the constraint is implicitly catchall and then the
constraint

```cheese
.{x: 0..3; y: constrain isEven; z: _} => ...
```

Or instead for tuple destructuring, it is instead `.(` that is used, and it instead is a list of constraints.
These are able to be nested as constraints to build patterns that match full trees.

#### Enum Destructuring Constraint

Same as the destructuring constraint, but an enum identifier is placed in front of the `{` or `(` rather than a `.`

```cheese

match node with {
  .empty => ...
  .integer(0..255 -> small_int) => ...
  .integer(_ -> big_int) => ...
  .add{lhs: constrain isConstant; rhs: constrain isConstant} => ...
  .add{lhs: _,rhs: _} => ...
  _ => ...
}
```

### Exhaustiveness

match statements must be exhaustive, meaning that all possible values must be covered, `constrain` constraints are by
default assumed to match *no* values, and as such alternatives to them must *always* be provided.

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

Continuing to the next loop in Cheese is done with the `continue` keyword

## For Loops

For loops in Cheese are created via the `for` keyword, followed by a value variable name (optionally prefixed w/ `*` or
`*~` for reference based values instead of copies), and then optionally a `,` and an index variable name followed by a
`:` and then the iterator, which can be a slice, an array, or an object that implements the `Iterable(T)` or
`ConstIterable(T)` interfaces, or defines the functions mandated within either of them. It can also be an expression
that returns an `Option(T)` and will loop until said function result is `.empty`, and it will automatically unwrap the
`Option(T)` to a value of type `T`. Or if the object implements the `Generator(T)` interface, or implements the
functions within

```cheese
for x : u32[0,1,2,3,4,5,6,7,8,9]
  ...
```

### For Transformations

After the iterable in a for loop, there may be a list of "transformations" which are either filters or maps to the
iterable, filters are declared with `? filter` and maps declared with `: map`

```cheese
// This will filter numbers that are prime and then square the aforementioned prime numbers
for x : NumbersUpTo(100) ? isPrime : square
  ...
```

### For Else

For else works in the same way as while else, if the loop finishes without yielding a value, it yields the else value

## `self`/`~self`/`Self` keywords

These are special keywords in Cheese used within structures used for function arguments/to refer to the type of the
structure.

```cheese
fn x self => void
  self.x = 0
```

### `self`/`~self`

The `self`/`~self` keywords when used as function arguments mark the function as a member function and also define a
self argument of type `*Self`/`*~Self` depending on whether `self` or `~self` was used respectively, with the name of
`self` that can be referred to within the function.

### `Self`

The `Self` keyword returns the containing structure of whatever scope you are within.

## Operators

Cheese defines the following operators with the following precedence

| Operator | Precedence | Description                       | Associativity |
|----------|------------|-----------------------------------|---------------|
| `()`     | 1          | Function Call/Tuple Construction  | Left-to-right |
| `[]`     | 1          | Array Index/Array Construction    | Left-to-right |
| `{}`     | 1          | Object Construction               | Left-to-right |
| `.`      | 1          | Object subscription               | Left-to-right |
| `+x`     | 2          | Unary plus                        | Right-to-left |
| `-x`     | 2          | Unary minus                       | Right-to-left |
| `[]x`    | 2          | Array Type Creation               | Right-to-left |
| `[]~`    | 2          | Constant Array Type Creation      | Right-to-left |
| `<>`     | 2          | Slice Type Creation               | Right-to-left |
| `<>~`    | 2          | Constant Slice Type Creation      | Right-to-left |
| `&x`     | 2          | Address of                        | Right-to-left |
| `$`      | 2          | Dereference                       | Left-to-right |
| `not`    | 2          | Boolean/bitwise not               | Right-to-left |
| `*x`     | 2          | Reference Type Creation           | Right-to-left |
| `*~`     | 2          | Constant Reference Type Creation  | Right-to-left |
| `*`      | 3          | Multiplication                    | Left-to-right |
| `/`      | 3          | Division                          | Left-to-right |
| `%`      | 3          | Remainder                         | Left-to-right |
| `+`      | 4          | Addition                          | Left-to-right |
| `-`      | 4          | Subtraction                       | Left-to-right |
| `<<`     | 5          | Left shift                        | Left-to-right |
| `>>`     | 5          | Right shift                       | Left-to-right |
| `<`      | 6          | Lesser than                       | Left-to-right |
| `>`      | 6          | Greater than                      | Left-to-right |
| `<=`     | 6          | Lesser than or equal to           | Left-to-right |
| `>=`     | 6          | Greater than or equal to          | Left-to-right |
| `==`     | 7          | Equal to                          | Left-to-right |
| `!=`     | 7          | Not equal to                      | Left-to-right |
| `and`    | 8          | Bitwise/boolean and               | Left-to-right |
| `xor`    | 9          | Bitwise/boolean xor               | Left-to-right |
| `or`     | 10         | Bitwise/boolean or                | Left-to-right |
| `&`      | 11         | Combination                       | Left-to-right |
| `:=`     | 12         | Reference reassignment            | Right-to-left |
| `=`      | 12         | Assignment                        | Right-to-left |
| `+=`     | 12         | Assignment by sum                 | Right-to-left |
| `-=`     | 12         | Assignment by difference          | Right-to-left |
| `*=`     | 12         | Assignment by multiplication      | Right-to-left |
| `/=`     | 12         | Assignment by division            | Right-to-left |
| `%=`     | 12         | Assignment by remainder           | Right-to-left |
| `<<=`    | 12         | Assignment by left-shift          | Right-to-left |
| `>>=`    | 12         | Assignment by right-shift         | Right-to-left |
| `and=`   | 12         | Assignment by boolean/bitwise and | Right-to-left |
| `xor=`   | 12         | Assignment by boolean/bitwise or  | Right-to-left |
| `or=`    | 12         | Assignment by boolean/bitwise xor | Right-to-left |

### Overloading

A lot the operators in Cheese can be overloaded by objects, this is by defining a function with the `operator` keyword
instead of the `fn` keyword with the name being the name being the name of the operator in the table above. More
accurate descriptions of how to overload them will be provided in each of the subsections for the operators.

Note: If the self argument is omitted from an overload, it then becomes an overload on the structure itself rather than
an object of the structure, which in this case the `.`, `$`, or any of the assignment operators can not be overloaded

Overloading the `()`, `[]` or `{}` operators on a structure are a way to make a constructor like syntax. And the object
can still be constructed normally with the `.{}` object constructor and in inferred type.

### `()`

`()` is both the function call operator, and the tuple construction operator, it can take any number of arguments within
the parentheses which will be used to specialize which template is called if the object being called is a template.

#### Example

```cheese
let x = tup(1,2,3)
function(a,b,c,d)
```

#### Overloading

Overloading of this function is defined via the `operator ()` overload

```cheese
operator () self...,args... => return_type...
```

### `[]`

`[]` is both the array index operator, and the array construction operator, it can take any number of arguments within
the brackets.

#### Example

```cheese
let arr = u32[1, 2, 3, 4]
let arr2 = u32[.[1,2],.[3,4],.[5,6]]
let x = arr[1]
let y = arr2[0,1]
```

#### Overloading

Overloading of this operator is defined via the `operator []` overload.

```cheese
operator [] self...,args... => return_type...
```

### `{}`

`{}` is the structure construction operator, it takes a list of `field_name: value` within the braces.

#### Example

```cheese
let x = struc{a: 5, b: 3, c: 2}
```

#### Overloading

`{}` can be overloaded via the `operator {}` overload, as for the arguments, it takes a single object of a structural
type, an any type argument which is what the structure literal is converted into, or a list of named arguments that the
structure literal is destructured into.

```cheese
operator {} self, struc: any => return_type...
```

### `.`

`.` is the object subscription operator, it is quite similar to the C++ `->` operator in sense over overloading and such

#### Example

```cheese
let x = z.y
```

#### Overloading

`.` can be overloaded with the `operator .` keyword, this cannot take any arguments other than the self argument, and
must return a reference to another object which is what the subscription will go to.

```cheese
operator . self => *return_type...
```

### `+x`

`+x` is the unary plus operator in Cheese, by default on numbers it doesn't really do anything, it is added for symmetry
with the unary minus operator

#### Example

```cheese
let x = +1
```

#### Overloading

`+x`  can be overloaded with the `operator +x` keyword, this cannot take any arguments other than the self argument.

```cheese
operator +x self => Self
```

### `-x`

`-x` is the unary minus operator in Cheese, it takes a number and returns its additive complement, if used on an
unsigned integer, it causes a compiler error.

#### Example

```cheese
let x = -1
```

#### Overloading

`-x`  can be overloaded with the `operator -x` keyword, this cannot take any arguments other than the self argument.

```cheese
operator -x self => Self
```

### `[]x`

`[]x` is the array type creation in Cheese, it can take any number of arguments inside the `[]` to make a
multidimensional array. If any of the arguments inside the `[]` are `?` then those sizes are inferred from the creation
of the object that this type is being bound to.

#### Example

```cheese
let ty = [8,8,8]u32
```

#### Overloading

This operator cannot be overloaded.

### `[]~`

`[]x` is the immutable array type creation in Cheese, it can take any number of arguments inside the `[]` to make a
multidimensional array. If any of the arguments inside the `[]` are `?` then those sizes are inferred from the creation
of the object that this type is being bound to.

#### Example

```cheese
let ty = [8]~u32
```

#### Overloading

This operator cannot be overloaded.

### `<>`

`<>` is the slice type creation in Cheese, it does not take any arguments and produces a 1 dimensional slice.

#### Example

```cheese
let ty = <>u32
```

#### Overloading

This operator cannot be overloaded.

### `<>~`

`<>~` is the immutable slice type creation in Cheese, it does not take any arguments and produces a 1 dimensional slice.

#### Example

```cheese
let ty = <>~u32
```

#### Overloading

This operator cannot be overloaded.

### `&x`

`&x` is the address of operator in Cheese, it does not take any arguments, and returns the address of the variable
containing the object

#### Example

```cheese
let z = &x
```

#### Overloading

This operator cannot be overloaded.

### `$`

`$` is the postfix dereference operator in Cheese, it does not take any arguments, and returns the object contained at
the address the reference refers to

#### Example

```cheese
let x2 = z$
```

#### Overloading

`$` can be overloaded with the `operator $` operator, it takes no arguments than self.

```cheese
operator $ self => SomeType ...
```

### `not`

`not` is both the boolean and bitwise not operator in Cheese, it infers whether it is boolean or bitwise
depending on the type of the argument.

#### Example

```cheese
let False = not true
let inverse = (not x) + 1
```

#### Overloading

`not` can be overloaded with the `operator not` operator, it takes no arguments other than self.

```cheese
operator not self => Self ...
```

### `*x`

`*x` is the reference type creation operator.

#### Example

```cheese
let ty = *x
```

#### Overloading

This operator cannot be overloaded.

### `*~`

`*~` is the immutable reference type creation operator.

#### Example

```cheese
let ty = *~x
```

#### Overloading

This operator cannot be overloaded.

### `*`

`*` is the multiplication operator in Cheese.

#### Example

```cheese
let x = 5 * 3 // 15
```

#### Overloading

This operator can be overloaded with the `operator *` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator * self, other: *Self => Self ...
```

#### `*` Between a `comptime_string` And a Comptime Known Integer

The `*` operator can be used to repeat a `comptime_string` value if the other side is a comptime known integer.

#### `*` Between Array Values and Comptime Known Integers

The `*` operator can be used to repeat an array value by a comptime known integer. It will repeat the array by creating
a new array with the same dimensions of the prior except for the first which is what will be repeated.

### `/`

`/` is the division operator in Cheese.

#### Example

```cheese
let x = 15 / 3 // 5
```

#### Overloading

This operator can be overloaded with the `operator /` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator / self, other: *Self => Self ...
```

### `%`

`%` is the remainder operator in Cheese.

#### Example

```cheese
let x = 5 % 3 // 2
```

#### Overloading

This operator can be overloaded with the `operator %` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator % self, other: *Self => Self ...
```

### `+`

`+` is the addition operator in Cheese.

#### Example

```cheese
let x = 5 + 3 // 7
```

#### Overloading

This operator can be overloaded with the `operator +` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator + self, other: *Self => Self ...
```

#### `+` Between 2 Array Values of The Same Type And Dimensionality, With Known Sizes

The `+` operator can also be used to concatenate at comptime or runtime 2 array values with the same amount of
dimensions and with the dimensions after the first one having the same size, to create a new array where the first
dimension has a length of the sum of the first dimensions of the 2 arguments, and it is the first one concatenated with
the second one.

#### `+` w/ `comptime_string` values

The `+` operator can be used at comptime to concatenate 2 `comptime_string` values into a new string.

### `-`

`-` is the subtraction operator in Cheese.

#### Example

```cheese
let x = 5 - 3 // 2
```

#### Overloading

This operator can be overloaded with the `operator -` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator - self, other: *Self => Self ...
```

### `<<`

`<<` is the left shift operator in Cheese.

#### Example

```cheese
let x = 5 << 3 // 40
```

#### Overloading

This operator can be overloaded with the `operator <<` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator << self, other: *Self => Self ...
```

### `>>`

`>>` is the right shift operator in Cheese.

#### Example

```cheese
let x = 15 >> 3 // 1
```

#### Overloading

This operator can be overloaded with the `operator >>` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator >> self, other: *Self => Self ...
```

### `<`

`<` is the relational operator that checks if the lhs is less than the rhs in Cheese.

#### Example

```cheese
let x = 15 < 3 // false
```

#### Overloading

This operator can be overloaded with the `operator <` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator < self, other: *Self => bool ...
```

### `>`

`>` is the relational operator that checks if the lhs is greater than the rhs in Cheese.

#### Example

```cheese
let x = 15 > 3 // true
```

#### Overloading

This operator can be overloaded with the `operator >` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator > self, other: *Self => bool ...
```

### `<=`

`<=` is the relational operator that checks if the lhs is less than or equal to the rhs in Cheese.

#### Example

```cheese
let x = 15 <= 3 // false
```

#### Overloading

This operator can be overloaded with the `operator <=` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator <= self, other: *Self => bool ...
```

### `>=`

`>=` is the relational operator that checks if the lhs is greater than or equal to the rhs in Cheese.

#### Example

```cheese
let x = 15 >= 3 // true
```

#### Overloading

This operator can be overloaded with the `operator >=` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator >= self, other: *Self => bool ...
```

### `==`

`==` is the relational operator that checks if the lhs is equal to the rhs in Cheese.

#### Example

```cheese
let x = 15 == 3 // false
```

#### Overloading

This operator can be overloaded with the `operator ==` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator == self, other: *Self => bool ...
```

### `!=`

`!=` is the relational operator that checks if the lhs is equal to the rhs in Cheese.

#### Example

```cheese
let x = 15 != 3 // true
```

#### Overloading

This operator can be overloaded with the `operator !=` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator != self, other: *Self => bool ...
```

### `and`

`and` is the bitwise/boolean and operator in Cheese, it infers which one it is by the type of its left and right hand
sides. It obeys short-circuiting rules when used as a boolean operator.

#### Example

```cheese
let x = 5 and 3 // 1
let y = false and true // false
```

#### Overloading

This operator can be overloaded with the `operator and` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator and self, other: *Self => Self...
```

### `xor`

`xor` is the bitwise/boolean xor operator in Cheese, it infers which one it is by the type of its left and right hand
sides.

#### Example

```cheese
let x = 5 xor 3 // 6
let y = false xor true // true
```

#### Overloading

This operator can be overloaded with the `operator xor` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator xor self, other: *Self => Self...
```

### `or`

`or` is the bitwise/boolean or operator in Cheese, it infers which one it is by the type of its left and right hand
sides. It obeys short-circuiting rules when used as a boolean operator.

#### Example

```cheese
let x = 5 or 3 // 7
let y = false or true // true
```

#### Overloading

This operator can be overloaded with the `operator or` operator, it takes one argument other than self, and that
argument can be of any type.

```cheese
operator or self, other: *Self => Self...
```

### `&`

`&` is the interface/concept combination operator, it works as described in the relevant sections.

#### Example

```cheese
let NumberFunction = Numeric & Function(void)
```

#### Overloading

This operator can not be overloaded.

#### `&` Between Two `enum` Types.

The `&` operator when used between two `enum` types creates a new enum type that is the combination of its left and
right hand side, this only works if both enumerations don't have properties with the same name with different field type
declarations, and if there are no specific values declared in either enumeration. Any value of the 2 enumerations can be
implicitly converted into the combined enumeration, but the dynamic cast operator `@*` must be used to convert the
combined enum values back into the original enumeration.

### `:=`

`:=` is the reference re-assignment operator in Cheese, it is used to redefine what a reference points to, as
opposed to `=` on a reference which instead uses `operator =`

#### Example

```cheese
let x mut = &y
x := z // Make x point to z instead of y
```

#### Overloading

This operator can not be overloaded.

### `=`

`=` is the assignment operator in Cheese, it is used when changing the value of a mutable object

#### Example

```cheese
let x: i32 mut = 5
x = 7 // Change the value of x to 7
```

#### Overloading

This operator can be overloaded with the `operator =` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator = self, other: OtherType => void ...
```

### `+=`

`+=` is the assignment by addition operator in Cheese, it is used when changing the value of a mutable object by
performing addition between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x += 7 // Change the value of x by adding 7 to it
```

#### Overloading

This operator can be overloaded with the `operator +=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator += self, other: OtherType => void ...
```

### `-=`

`-=` is the assignment by difference operator in Cheese, it is used when changing the value of a mutable object by
performing subtraction between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x -= 7 // Change the value of x by subtracting 7 from it
```

#### Overloading

This operator can be overloaded with the `operator -=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator -= self, other: OtherType => void ...
```

### `*=`

`+=` is the assignment by multiplication operator in Cheese, it is used when changing the value of a mutable object by
performing multiplication between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x *= 7 // Change the value of x by multiplying it by 7
```

#### Overloading

This operator can be overloaded with the `operator *=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator *= self, other: OtherType => void ...
```

### `/=`

`/=` is the assignment by division operator in Cheese, it is used when changing the value of a mutable object by
performing division between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x /= 7 // Change the value of x by dividing it by 7
```

#### Overloading

This operator can be overloaded with the `operator /=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator /= self, other: OtherType => void ...
```

### `%=`

`%=` is the assignment by remainder operator in Cheese, it is used when changing the value of a mutable object by
performing division between the object and the other value and assigning the remainder to the value.

#### Example

```cheese
let x: i32 mut = 5
x %= 7 // Change the value of x by getting the remainder of dividing it by 7
```

#### Overloading

This operator can be overloaded with the `operator %=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator %= self, other: OtherType => void ...
```

### `<<=`

`<<=` is the assignment by left shift operator in Cheese, it is used when changing the value of a mutable object by
performing a left shifting between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x <<= 7 // Change the value of x by shifting it left by 7
```

#### Overloading

This operator can be overloaded with the `operator <<=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator <<= self, other: OtherType => void ...
```

### `>>=`

`>>=` is the assignment by right shift operator in Cheese, it is used when changing the value of a mutable object by
performing a right shift between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x >>= 7 // Change the value of x by shifting it right by 7
```

#### Overloading

This operator can be overloaded with the `operator >>=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator >>= self, other: OtherType => void ...
```

### `and=`

`+=` is the assignment by and operator in Cheese, it is used when changing the value of a mutable object by
performing a logical or bitwise and between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x and= 7 // Change the value of x by performing a bitwise and with it and 7
```

#### Overloading

This operator can be overloaded with the `operator and=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator and= self, other: OtherType => void ...
```

### `or=`

`or=` is the assignment by or operator in Cheese, it is used when changing the value of a mutable object by
performing a logical or bitwise or between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x or= 7 // Change the value of x by or-ing it with 7
```

#### Overloading

This operator can be overloaded with the `operator or=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator or= self, other: OtherType => void ...
```

### `xor=`

`xor=` is the assignment by exclusive or operator in Cheese, it is used when changing the value of a mutable object by
performing a logical or bitwise xor between the object and the other value.

#### Example

```cheese
let x: i32 mut = 5
x xor= 7 // Change the value of x by xoring it with 7
```

#### Overloading

This operator can be overloaded with the `operator xor=` operator. It takes one other argument than a mutable `self` and
must return `void`

```cheese
operator xor= self, other: OtherType => void ...
```

## `import`

To import other files into your project, you can use the `import` keyword in structure level declarations. Following the
import keyword is an import path, where it is a path separated by `.` and finally an optional `@ name` to change the
name of the imported module within the scope, by default this name is the last item on the import path. Imported modules
are of type `struct`

```cheese
import std.memory
```

### `import` Resolution

To resolve an import, first the compiler converts the path to a path as such: `.` -> `/`, `$` -> `..`
and everything else passes through. It then defines the following values to be used later in describing this process

| Value Name       | Description                                                                |
|------------------|----------------------------------------------------------------------------|
| `$path`          | The converted path                                                         |
| `$projectDir`    | The current project directory                                              |
| `$newProjectDir` | Is defined mid-resolution, the new project directory to use for the import |
| `$libraryPaths`  | The list of library paths                                                  |

It first checks if `$projectDir/$path.chs` exists, if it does, it imports it and defines `$newProjectDir` as the folder
containing `$projectDir/$path.chs`. It then checks if `$projectDir/$path/lib.chs` exists, and if it does, it imports it
and defines `$newProjectDir` as `$projectDir/$path/lib.chs`. Then for each `$libraryPath` in `$libraryPaths` it checks
if
`$libraryPath/$path.chs` exists, and if it does, it imports it and defines `$newProjectDir` as the folder containing
`$libraryPath/$path.chs`, and if it doesn't it checks if `$libraryPath/$path/lib.chs` exists, and if it does, it defines
`$newProjectDir` as `$libraryPath/$path` and imports it. At any point when the compiler has imported something, it then
stops the import resolution process.

## Note on Commas and Semicolons

Commas and semicolons are optional in Cheese where it can be inferred, and are generally not recommended to use unless
you want to put multiple statements on one line, like with say function calls or function declarations. `cheese fmt`
will eventually be able to detect if statements were seperated with commas/semicolons or not and put the statements on
multiple lines if not, the following subsections will describe the behaviour of commas and semicolons in every single
context they are recognized. Note in most cases you can put multiple statements on one line without separators and the
compiler will still recognize it correctly except in cases where it is ambiguous, `cheese fmt` will split these into
multiple lines how the compiler recognizes it. If a list of statements ends with a trailing comma or semicolon the
formatter will multiline it in that case as well.

### Top Level Declarations

In top level declarations, semicolons are recognized for putting multiple declarations on one line. And commas are used
for putting multiple fields in one line

```cheese
/* file with commas/semicolons */
a: i32 public, b: i32 public, c: i32 public
let A = 0; let B = 0; fn C => void {}


/* file without commas/semicolons*/
a: i32 public
b: i32 public
c: i32 public
let A = 0
let B = 0
fn C => void {}
```

### Function Declarations

In function declarations, commas are recognized to split the argument declarations up, semicolons are not recognized.

```cheese
/* function with commas */
fn fma a: f32, b: f32, c: f32 => f32
  $fusedMultiplyAdd(a,b,c)

/* function without commas */
fn fma
  a: f32
  b: f32
  c: f32
  => f32
    $fusedMultiplyAdd(a,b,c)
```

### Tuple Declarations

In tuple declarations, commas are recognized to split the field types up, semicolons are not recognized.

```cheese
/* tuple with commas */
tuple (i32, i32, i32)

/* tuple without commas */
tuple (
  i32
  i32
  i32
)
```

### Structure Declarations

Within structure declarations, commas and semicolons follow the same rules as within top level declarations

### Blocks

Within blocks, semicolons are recognized to split up individual statements.

### Structure Literals, Array Literals, Tuple Literals, Function Calls, Array Indices

Within the aforementioned constructs, commas are recognized to split up the individual expressions within them.

### Match Statements

Within match statement arms, commas are recognized to split up the different constraints
Within destructured match arms, semicolons are recognized to split up the fields, new lines do the same

------------------------------------------------------------------------------------------------------------------------

# Behaviours

## Peer Type Resolution

When trying to infer the type value of an expression or function with 2 or more differing types the compiler employs
something called peer type resolution where it attempts to find a mutual type to coerce them to. The table below shows
what this peer type will be between pairs of types. And it can be extrapolated to more than just pairs. If two types are
not shown on this table, then attempting to find the peer type between them will cause a compiler error

### Peer Type Table

<!---                                    V                                      V paddings must stay at these points for proper rendering--->

| Type A             | Type B             | Peer Type                                                                                                                                     |
|--------------------|--------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| `u(N1)`            | `u(N2)`            | `u(max(N1,N2)`                                                                                                                                |
| `i(N1)`            | `i(N2)`            | `i(max(N1,N2)`                                                                                                                                |
| `u(N1)`            | `i(N2)`            | `i(max(N1,N2)` but panics if `u(N1)` would cause a negative number to be represented when safety is enabled                                   |
| `u/i(N)`           | `f32`              | `f32`                                                                                                                                         |
| `u/i(N)`           | `f64`              | `f64`                                                                                                                                         |
| `u/i(N)`           | `c32`              | `c32`                                                                                                                                         |
| `u/i(N)`           | `c64`              | `c64`                                                                                                                                         |
| `f32`              | `f64`              | `f64`                                                                                                                                         |
| `f32`              | `c32`              | `c32`                                                                                                                                         |
| `f32`              | `c64`              | `f64`                                                                                                                                         |
| `f64`              | `c32`              | `c32`                                                                                                                                         |
| `f64`              | `c64`              | `c64`                                                                                                                                         |
| `comptime_int`     | `u(N)`             | `u(N)` but errors if the `comptime_int` is negative or is not representable within N bits                                                     |
| `comptime_int`     | `i(N)`             | `i(N)` but errors if the `comptime_int` is not representable within N bits                                                                    |
| `comptime_int`     | `f32`              | `f32`                                                                                                                                         |
| `comptime_int`     | `f64`              | `f64`                                                                                                                                         |
| `comptime_int`     | `c32`              | `c32`                                                                                                                                         |
| `comptime_int`     | `c64`              | `c64`                                                                                                                                         |
| `comptime_int`     | `comptime_float`   | `comptime_float`                                                                                                                              |
| `comptime_int`     | `comptime_complex` | `comptime_complex`                                                                                                                            |
| `comptime_float`   | `u(N)`             | `u(N)` but errors if the `comptime_float` is negative or not a representable integer within N bits                                            |
| `comptime_float`   | `i(N)`             | `i(N)` but errors if the `comptime_float` is not a representable integer within N bits                                                        |
| `comptime_float`   | `f32`              | `f32`                                                                                                                                         |
| `comptime_float`   | `f64`              | `f64`                                                                                                                                         |
| `comptime_float`   | `c32`              | `c32`                                                                                                                                         |
| `comptime_float`   | `c64`              | `c64`                                                                                                                                         |
| `comptime_float`   | `comptime_complex` | `comptime_complex`                                                                                                                            |
| `comptime_complex` | `c32`              | `c32`                                                                                                                                         |
| `comptime_complex` | `c64`              | `c64`                                                                                                                                         |
| `comptime_string`  | `[*](~)u8`         | `[*](~)u8` (with the string being null-terminated)                                                                                            |
| `comptime_string`  | `<>(~)u8`          | `<>(~)u8` (with the string not being null-terminated)                                                                                         |
| `comptime_string`  | `[...](~)u8`       | `[...](~)u8` but errors if the string is not the exact length stated in the one dimensional array (with the string not being null terminated) |
| `T`                | `Result(T,E)`      | `Result(T,E)` with the T being put into the .Ok form                                                                                          |
| `E`                | `Result(T,E)`      | `Result(T,E)` with the E being put into the .Err form                                                                                         |
| `T`                | `Optional(T)`      | `Optional(T)` with the T being put into the .Some form                                                                                        |
| `noreturn`         | `T`                | `T` if the types aren't part of a binary operation where the value can't be short-circuited otherwise `noreturn`                              |

## Comptime vs. Runtime

------------------------------------------------------------------------------------------------------------------------

# Builtins

## Concepts

### `Tuple`/`Tuple(N)`

These concepts match objects that are of a tuple type, where the specialization `Tuple(N)` matches an object of a tuple
type with N fields.

### `Function(A...,R)`

This concept matches an object that has a `()` operator that has the arguments `A...` and the return type `R`.

### `Numeric`

This concept matches an object that defines the following operator overloads

* `+`
* `-`
* `*`
* `/`
* `%`
* `>`
* `<`
* `==`
* `>=`
* `<=`
* `!=`
* `-x`
* `+x`

## Interfaces

Note, all interfaces defined here, unless stated otherwise, are not `dynamic` by default, to get the `dynamic` version,
it is the interfaces name with `Dynamic` put afterwards, so to get a `dynamic` version of `Callable(Args...,R)` you
instead use `CallableDynamic(Args...,R)`. Also note, all the `dynamic` versions are just thin extensions of the non
`dynamic` versions, so it will implicitly cast back down to the non `dynamic` one if need be.

### `Callable(Args...,R)`

Denotes a non-copyable, non-droppable interface that says this object is callable with the provided arguments

* Mandates an `operator() Args... => R` function

### `CallableObject(Args...,R)`

Denotes a function much the same as `Callable(Args...R)`, but implies copyable and droppable.

* Mandates an `operator() self, Args... => R` function
* Also mandates a `copy ~self => CallableObject(...)` function
* And a `drop self => void` function

### `Droppable`

Denotes that the object gets destroyed when it leaves scope.

* Mandates a `drop => void` function

### `Allocator`

Denotes an allocator.

* Todo `Allocator` mandates

------------------------------------------------------------------------------------------------------------------------

# Standard Library

TODO: Standard library documentation (and standard library)
------------------------------------------------------------------------------------------------------------------------