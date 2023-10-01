// A simple test file to test function pointer combination with anything
fn test_fn x: any => $Type(x) public
    x
let fn_pointer_integer public = $fnPtr(test_fn, .Type(i64))
let fn_pointer_float public = $fnPtr(test_fn, .Type(f64))
let fn_pointer_complex public = $fnPtr(test_fn, .Type(c64))
let fn_pointer_bool public = $fnPtr(test_fn, .Type(bool))