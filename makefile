default: main

main:
	clear 
	gcc -w -I/usr/local/include/X11 -g -o client client-chat.c -lgthread-2.0 `pkg-config gtk+-3.0 --cflags --libs`
	gcc -o server server.c -g -w -pthread 
	# -lcrypt

w:
	clear 
	gcc -Wall -g -o client client-chat.c -lgthread-2.0 `pkg-config gtk+-3.0 --cflags --libs`
	gcc -o server server.c -g -Wall -pthread 
	# -lcrypt

run: main
	./client 
debug: main
	gdb ./client	
server: main
	./server
s: server	
clean:
	rm -f client server