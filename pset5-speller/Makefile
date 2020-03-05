release:
	clang -O2 -std=c11 -c -o speller.o speller.c
	clang -O2 -std=c11 -c -o dictionary-hashtable.o dictionary-hashtable.c
	clang -O2 -std=c11 -c -o dictionary-prefixtree.o dictionary-prefixtree.c
	clang -O2 -std=c11 -o speller-ht speller.o dictionary-hashtable.o -lm
	clang -O2 -std=c11 -o speller-pt speller.o dictionary-prefixtree.o -lm
debug:
	clang -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o speller.o speller.c
	clang -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -c -o dictionary.o dictionary-hash-table.c
	clang -ggdb3 -O0 -Qunused-arguments -std=c11 -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wshadow -o speller speller.o dictionary.o -lm

