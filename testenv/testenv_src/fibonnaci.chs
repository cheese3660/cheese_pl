
fn main argc: u32, argv: [?,?]~u8 => i32 entry
{
    if argc < 2
    {
        _ = puts("usage: ")
        _ = puts(argv[0])
        _ = puts(" number\n")
        ==> 1
    }
    let num: u64 = atoi(argv[1]) @ u64
    fibonnaci(num)

    ==> 0
}

fn fibonnaci n: u64 => void
{
    // Lets do a simple iterative approach
    let cur: u64 mut = 2
    let a: u64 mut = 0
    let b: u64 mut = 1
    let c: u64 mut = a + b
    if n >= 1
    {
        _ = puts("1: ")
        _ = put_int(a)
        _ = putchar('\n')
    }
    if n >= 2
    {
        _ = puts("2: ")
        _ = put_int(b)
        _ = putchar('\n')
    }
    while cur < n
    {
        _ = put_int(cur+1)
        _ = puts(": ")
        _ = put_int(c)
        _ = putchar('\n')
        a = b
        b = c
        c = a + b
        cur = cur + 1
    }
}


fn put_int i: u64 => void
{
    let i_copy mut = i
    if i_copy == 0
    {
        _ = puts("0")
        ==> none
    }
    let digits: [?]~u8 = "0123456789"
    def chars: [20]u8 mut
    let idx: u64 = 0
    while i_copy != 0
    {
        chars[idx] = digits[i_copy % 10]
        i_copy = i_copy / 10
        idx = idx + 1
    }
    while idx != 0
    {
        _ = putchar(chars[idx])
        idx = idx - 1
    }
    _ = putchar(chars[idx])
}

fn puts s: [?]~u8 => i32 public import
fn putchar c: u8 => i32 public import
fn atoi s: [?]~u8 => i32 public import