fn main => u8 entry {
    _ = fact(3 @ u16)
    _ = fact(3 @ i17)
    _ = fact(4 @ u16)
    ==> fact(32 @ u8)
}
fn predecessor x: any => $Type(x)
    x-1

fn multiply x: any, y: any => $Peer($Type(x),$Type(y))
    x*y

fn fact x: any => $Type(x) ==> if x == 0 then 1 else multiply(x,fact(predecessor(x)))