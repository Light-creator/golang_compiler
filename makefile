all: compiler recognizer

recognizer:
	bison -d go_parser.y
	flex -o lex_rec.yy.c lex_rec.l
	gcc go_parser.tab.c lex_rec.yy.c -o recognizer

compiler:
	bison -d parser.y
	flex lex.l
	gcc parser.tab.c lex.yy.c -o compiler
	gcc vm.c -o vm -g

clean:
	rm ./vm
