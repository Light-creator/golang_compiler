package main

import "fmt"

func main() {
    a := 100
		
		if a >= 15 + 10 {
			x := 13

			for x < 17 {
				x++
			}

			fmt.Println(x)

			if x < 20 {
				fmt.Println(200)
			}

			fmt.Println(123)
		}

		fmt.Println(a)
}
