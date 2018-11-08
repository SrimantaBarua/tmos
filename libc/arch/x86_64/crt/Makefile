all: crti.o crt0.o crtn.o

clean:
	rm -f crti.o crt0.o crtn.o

%.o: %.asm
	nasm -felf64 -o $@ $^
