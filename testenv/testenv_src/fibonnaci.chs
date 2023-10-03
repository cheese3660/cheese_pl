
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
    if num == 0
    {
        _ = puts("number must be > 0")
        ==> 1
    }
    if num >= 94
    {
        _ = puts("number must be <= 93")
        ==> 1
    }
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
    def chars: [21]u8 mut
    _ = puts(_ui64toa(i, chars,10))
}

fn puts s: [?]~u8 => i32
{
    let i: u64 mut = 0
    while s[i] != 0
    {
        _ = putchar(s[i])
        i = i+1
    }
    ==> 0
}
fn putchar c: u8 => i32 public import
fn atoi s: [?]~u8 => i32 public import
fn _ui64toa value: u64, buffer: [?]u8, radix: i32 => [?]~u8 public import