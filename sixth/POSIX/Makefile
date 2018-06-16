all:
	gcc -lrt server.c header.c -o server
	gcc -lrt client.c header.c -o client

clean:	cleanServer cleanClient mqueue

mqueue:
	rm /dev/mqueue/*

cleanServer:
	rm server

cleanClient:
	rm client