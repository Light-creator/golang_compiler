package main

import "fmt"

func main() {
    a := -10
    b := 5
    c := -3
    
    result := 0
    
    if a < 0 {
        if b > 0 {
            result = a * b
        } else {
            result = a + b
        }
    } else {
        result = c - b
    }
    
    fmt.Println(result)
    
		i := -5
    for i < 0 {
        x := i * 2
        fmt.Println(x)
				i++
    }
}
