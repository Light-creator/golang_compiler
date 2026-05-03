all:
	bison -d parser.y
	flex lex.l
	gcc parser.tab.c lex.yy.c -o main
	gcc vm.c -o vm -g

clean:
	rm ./vm
