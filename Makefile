COMPILER=clang
C_FLAGS=-Wall -Wextra -g -ggdb

pixie: pixie.c
	$(COMPILER) $(C_FLAGS) -o pixie pixie.c -lm

art: art.c
	$(COMPILER) $(C_FLAGS) -o art art.c -lm

clean: 
	rm pixie

