#CC = cc 				
CC = gcc  				
COMPILER = $(CC)
CFLAGS = -w -o
PORT = 3543
DROPRATE = 15
#DROPRATE = 50

all : client.c server.c
	$(COMPILER) $(CFLAGS) client client.c
	$(COMPILER) $(CFLAGS) server server.c
	
runC : client
	./client $(PORT)
	
runS : server
	./server $(PORT) $(DROPRATE)
	
clean:
	@echo "CLEANING"
	rm client
	rm server
	rm *.o
