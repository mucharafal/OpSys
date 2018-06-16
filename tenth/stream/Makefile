all: stream_server stream_client

Options=-g

stream_client: stream_client.o stream_connection_lib.o
	gcc stream_client.o stream_connection_lib.o -o stream_client

debug: stream_client.o stream_connection_lib.o stream_server.o
	gcc stream_server.o stream_connection_lib.o -g -pthread
	gcc stream_client.o stream_connection_lib.o -g 

stream_server: stream_server.o stream_connection_lib.o
	gcc stream_server.o stream_connection_lib.o -o stream_server -pthread