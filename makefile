default: run

main:
	clear 
	gcc -w -I/usr/local/include/X11 -g -o client client-chat.c -lgthread-2.0 `pkg-config gtk+-3.0 --cflags --libs`

run: main
	./client 
debug: main
	gdb ./client

clean:
	rm -f main