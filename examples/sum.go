package main

import "fmt"

func main() {
    sum := 0
    
		i := 0
    for i < 5 {
				
				j := 0
        for j < 3 {
            if i > 2 {
                if j == 1 {
                    sum = sum + i * j
                } else {
                    sum = sum + 1
                }
            } else {
                sum = sum + j
            }

						j++
        }

				i++
    }
    
    fmt.Println(sum)
}
