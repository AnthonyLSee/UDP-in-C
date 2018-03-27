Reliable Unicast Using UDP________  

This program will demonstrate reliable unicast using UDP. There are two c files, client and server.

client.c / SENDER
    This file is the sender file. It takes in a port number as its argument. Note the program will
    default the host IP to the local host / 127.0.0.1, so make sure to run the server.c file in another
    terminal.
    
    This program will send packets over to server.c and wait for server.c to ACK. The packets are created through
    user input. The program has a timeout whenever the server takes too long (2 seconds). If timeout, then the program
    will go into a temp loop. It will keep sending over the same packet until accepted and ACK.

server.c / RECEIVER
    This file is the receiver file. It takes a port number and a number between 0-100 which will act as the drop rate. The drop
    rate will emulate an unreliable connection.
    
    This program will receive packets. Whenever it successfully receives a packet it will send over an ACK to the sender. The drop
    rate will create an unreliable connection, which will just discard the packet and wait for the sender. It also builds a file
    with all the packets from the sender inside recvFile.txt.

Running the programs
    There is a makefile that will both compile everything. Make sure to run the makefile like this:

	make all : Make all the files
	make runS : Runs Server
	make runC : Runs Clinet
	make clean : Cleans up files.
    

How to run the programs:
1. Make sure sendFile and recvFile exists. If you are going to use your own file, please rename them to these.
2. Run Server First
	./server <PORT> <DROP RATE>     // Drop rate : 0 to 100:
3. Run Client
    ./client <PORT>
	
COMMENTS:
	the FIRST run of client could be a bit buggy. Everytime I ran client on a fresh server, it will always drop the first
	packet but if I run client again, it won't drop and run faster. I believe this is due to how drand() operates. 
	While the first packet will always drops, it still should demonstrate the protocols.
	














