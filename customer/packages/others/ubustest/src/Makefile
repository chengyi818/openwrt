all : server client

server : server.o count.o
	$(CC) -o server server.o count.o

client : client.o count.o
	$(CC) -o client client.o count.o

#server.o : server.c count.h 
#	$(CC) -c server.c
#client.o : client.c count.h
#	$(CC) -c client.c
#count.o : count.c count.h
#	$(CC) -c count.c

.PHONY : clean all
clean :
	-rm server client server.o client.o count.o
