# Room-Booking-System
In this repository, one can find the client and server source code for a simple room booking system. The server is a concurrent, multi-process server. 

Tester.sh is not working as expected, however the other programs work well.

To Run the program  
1) Compile the server with `gcc MultiProc_server.c -o server` and run with `./server`  
2) Compile the client with `gcc client.c -o client`  
3) Run the client as `./cliner <path to input csv> <path to output csv>`  

Note that some line breaks cause issues while running. Please have line break as `\n`.  