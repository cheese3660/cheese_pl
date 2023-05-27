fn abs arg: f64 => f64
    if arg < 0
        -arg
    else
        arg

let HSL=struct{
    h:f64
    s:f64
    l:f64
    fn to_rgb ~self=>RGB public{
        let c = (1 - abs(2.0 * self.l - 1.0)) * self.s
        let x =c * (1 - abs((self.h / 60.0) % 2.0 - 1.0))
        let m = self.l - c/2.0
        let h60 = (self.h / 60.0) @ u8
        let(rprime, gprime, bprime)=match h60
        {
            0 =>
                .(c,x,0.0 @ f64)
            1 =>
                .(x,c,0.0 @ f64)
            2 =>
                .(0.0 @ f64,x,c)
            3 =>
                .(x,0.0 @ f64,c)
            4 =>
                .(c,0.0 @ f64,x)
            _ =>
                .(0.0 @ f64,0.0 @ f64,0.0 @ f64)
        }
        ==>.{
                r: ((rprime+m)*255)@u8
                g: ((gprime+m)*255)@u8
                b: ((bprime+m)*255)@u8
            }
    }
}

let RGB=struct{
    r:u8
    g:u8
    b:u8
}

fn main=>u8 entry{
    let hsl=HSL{
        h:130.0
        s:0.5
        l:0.5
    }

    _ = hsl.to_rgb()

    ==>0
}