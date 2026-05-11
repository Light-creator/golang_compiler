package main

import "fmt"

func main() {
    x := 15
    y := 7
    z := 3
    
    result1 := x + y*z - (x - y) / z
    fmt.Println(result1)
    
    if (x > y) && (y > z) {
        result2 := x*y - z*10 + 5
        fmt.Println(result2)
    }
}
