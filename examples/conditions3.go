package main

import "fmt"

func main() {
    a := 5
    b := 10
    c := 15
    
    if a < b && b < c || a == 5 {
        d := 100
        fmt.Println(d)
    }
    
    if !(a > b) && (c > b || a == 0) {
        e := 200
        fmt.Println(e)
    }
}
