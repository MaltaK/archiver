all:
	gcc arch.c -o bin
san:
	gcc -fsanitize=address arch.c -o bin
