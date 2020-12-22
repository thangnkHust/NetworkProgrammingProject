default: main

main: clean
	clear
	gcc -w -I/usr/local/include/X11 -g -o client client-chat.c -lgthread-2.0 `pkg-config gtk+-3.0 --cflags --libs`
	gcc -o server server.c -g -w -pthread 

run: 
	./client 

server: main
	./server

clean:
	rm -f client server