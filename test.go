package main

import "fmt"

func main() {
    a := 100
		i := 0

		for i != 20 {
			i += 5
			fmt.Println(i)
		}

		fmt.Println(a)
}
