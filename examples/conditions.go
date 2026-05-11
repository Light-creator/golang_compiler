package main

import "fmt"

func main() {
    a := 10
    b := 20
    c := 30
    
    if a < b && b < c {
        result := a + b*c
        fmt.Println(result)
    }
    
    if a > 5 || b > 25 {
        d := a - b
        fmt.Println(d)
    }
    
    x := a * (b - c)
    fmt.Println(x)
}
