COMPILER=clang
C_FLAGS=-Wall -Wextra -g -ggdb

all: pixie

pixie: pixie.c
	$(COMPILER) $(C_FLAGS) -o pixie pixie.c -lm

clean: 
	rm pixie

