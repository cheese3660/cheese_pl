fn main => i64 entry
    fact(3)

fn fact x: i64 => i64
    if x == 0
        1
    else
        x*fact(x-1)