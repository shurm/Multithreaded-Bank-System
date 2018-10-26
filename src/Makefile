all: target1 target2

target1: 
	gcc -Wall -Werror -pthread server.c -o server
target2: 
	gcc -Wall -Werror client.c -o client


clean:
	rm server client

