package main

import "fmt"

func main() {
    a := 100
		i := 0

		for i != 20 {
			if i < 15 && a == 100 {
				if i > 12 {
					fmt.Println(i)
				} else {
					x := a - i
					fmt.Println(x)
				}
			}
			
			i++
		}

		fmt.Println(a)
}
