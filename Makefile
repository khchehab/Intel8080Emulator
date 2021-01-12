all: clean main
	@./out/main

main: out
	@clang src/main.c src/i8080.c -o out/main --std=c11

out:
	@mkdir -p out

clean:
	@rm -rf out
