// This should be an example of the canonical hello world

import std
import std.io //Importing std already does this but this gives us directly the `io` struct

// For hosted applications, std sets up the entry point to use a vector if the main function uses that type for the argument
// Other formats accepted by the std entry point
// arguments: <>~(<>~u8)
// nargs: usize, arguments: [*]~([*]~u8)
// nargs: usize, arguments: [*]~(<>~u8)
// arguments: <>~([*]~u8)
// arguments: std.collections.vector([*]~u8)
// When the base string type is a [*]~u8 then it is null terminated.
// The result of main can also be an error type.
fn main arguments: std.collections.vector(<>~u8) => u8
{
    // Explicitly disregard the error output of io.stdout.write, as we do not care
    _ = io.stdout.write("Hello, world!")
    ==> 0
}