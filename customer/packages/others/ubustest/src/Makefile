server : server.o count.o
	cc -o server server.o count.o

client : client.o count.o
	cc -o client client.o count.o

server.o : server.c count.h 
	cc -c server.c
client.o : client.c count.h
	cc -c client.c
count.o : count.c count.h
	cc -c count.c

clean :
	rm server client server.o client.o count.o
