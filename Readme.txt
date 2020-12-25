*******************************************************
*  Name      :  Biniyam Yohannes       
*  Student ID:  109697526               
*  Class     :  CSC 3761          
*  Lab#      :  1                
*  Due Date  :  Oct. 8, 2020
*******************************************************


                 Read Me


*******************************************************
*  Description of the program
*******************************************************

This program includes creates a client-server connection
between two computers and exchanges files between 
the two machines.


*******************************************************
*  Source files
*******************************************************

Name:  server.c
   Server side program. This program creates a server that
   will serve requests from the clients. It can also accept
   files from the cleint.

Name:  client.c
   Client side program. This program creates a client that can
   send requests to the server. It can also send files to the
   server.
   
*******************************************************
*  Circumstances of programs
*******************************************************

   The program runs successfully.  
   
   The program was compiled, run, and tested on 
   gcc csegrid.ucdenver.pvt and another linux machine.


*******************************************************
*  How to build and run the program
*******************************************************

1. Uncompress the byohannes_Lab1 file.    
   To uncompress it use the following commands 
       % unzip byohannes_Lab1

   Now you should see a directory named byohannes_Lab1 with the directories:
     	server (contains server.c and its makefile)
     	client (contains client.c and its makefile)

2. Build the program.

    Change to the directory that contains the files by:
    % cd byohannes_Lab1 
    and then either
    % cd client 
    or
    % cd server 

    Compile the programs by:
    % make

3. First run the server by:
   % ./server

4. Run the client by:
   % ./client [IP address] 24000
   (the port number 24000 is hardcoded and needs to be the same every time)

4. Delete the obj files, executables, and core dump by
   % make clean
