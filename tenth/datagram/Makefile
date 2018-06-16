all: datagram_server datagram_client

Options=-g

datagram_client: datagram_client.o datagram_connection_lib.o
	gcc datagram_client.o datagram_connection_lib.o -o datagram_client

debug: datagram_client.o datagram_connection_lib.o datagram_server.o
	gcc datagram_server.o datagram_connection_lib.o -g -pthread
	gcc datagram_client.o datagram_connection_lib.o -g 

datagram_server: datagram_server.o datagram_connection_lib.o
	gcc datagram_server.o datagram_connection_lib.o -o datagram_server -pthread