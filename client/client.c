#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>

//Close the connection
void bye(int descriptor)
{
	printf("Client closed connection.\n");
	close (descriptor);
	exit(1);
}

//Receive catalog
void cat(int descriptor, char *newBuffer)
{
	//Receive error
	if(recv(descriptor, newBuffer, 1000, 0) == -1)
		perror("recv");
	else
		printf("%s\n", newBuffer);
}

//Receive download
void download(int descriptor, char *bufferI)
{	
	//Receive error
	if(recv(descriptor, bufferI, 1000, 0) == -1)
			perror("recv");
	//File was not found
	if(strncmp(bufferI, "0", 1) == 0)
		printf("The file was not found.\n\n");
	//File was found
	else if(strncmp(bufferI, "1 ", 2) == 0)
	{
		//File size
		strtok(bufferI, " ");
		char *size = strtok(NULL, "\n");
		int x;
		sscanf(size, "%d", &x);
		memset(bufferI, 0, 1000);

		//File name
		if(recv(descriptor, bufferI, 1000, 0) == -1)
			perror("recv");

		FILE *fp = fopen(bufferI, "wb");
		char msg[1000];
		int quotient = x / sizeof(msg); 	//
		int remainder = x % sizeof(msg);	//
		char last[remainder];
		memset(msg, 0, 1000);
		memset(last, 0, remainder);

		//Receive the bulk of the message
		for(int i=0; i < quotient; i++)
		{
			if(recv(descriptor, msg, sizeof(msg), 0) == -1)
				perror("recv");
			else
			{
			  int rc; 
			  rc = fwrite(msg, 1, 1000, fp); 	//total number of elements successfully written, BY - swapped 1 and 100
			}
		}

		//Reeceive the remainder
		if(recv(descriptor, last, sizeof(last), 0) == -1)
				perror("recv");
		else
		{
		  int rc; //DMO
		  rc = fwrite(last, 1, remainder, fp); // DMO, BY - swapped 1 and remainder
		}

		//Close the file
		fclose(fp);

		//Success message
		memset(bufferI, 0, 1000);
		memset(msg, 0, sizeof(msg));
		memset(last, 0, remainder);
		printf("Received the entire file successfully.\n\n");
	}	
}


//Send file to server
void sendFile(FILE *filePtr, int fileLen, int descriptor)
{
	char *buffer;
	buffer = (char *) malloc(fileLen);
	char symbol;
	char msg[1000];
	memset(buffer, 0, sizeof(buffer));

	//Loop over the file data
	int i;
	for (i=0; i<fileLen; i=i+1000){
	  int rc;
	  rc = fread(buffer, 1, 1000, filePtr);		//total number of elements successfully read
	  rc = send(descriptor, buffer, rc, 0);		//number of bytes sent
	  // read in a 1000 bytes and send them
	}

	printf("File sent successfully.\n\n");

	//Free the memory
	memset(buffer, 0, sizeof(buffer));
	free(buffer); 
	fclose(filePtr);
}

//Upload file
void upload(int descriptor, char *buffer)
{
	printf("I got here 1.\n");
	strtok(buffer, " ");
	const char *filename = strtok(NULL, "\n");
	char temp[1000];
	memset(temp, 0, sizeof(temp));
	printf("I got here 2.\n");

	//Return control if file name is null
	if(filename == NULL)
	{
		printf("ERROR. Missing file name argument.\n\n");
		return;
	}

	strcpy(temp, filename);
	strcpy(buffer,temp);


	char msg[1000]; 	//MOVED THIS LINE INSIDE THE IF
	memset(msg, 0, sizeof(msg));
	if(recv(descriptor, msg, 1000, 0) == -1)
		perror("recv");
	else
		if(strncmp(msg, "1", 1) == 0)
			printf("The file is already in the target directory - overwriting file...\n");
	memset(msg, 0, sizeof(msg));

	//SAME AS DOWNLOAD FROM SERVER
	//Send file
	if (access(("./%s",buffer), F_OK) == 0)
	{
		//Send 'file found' and 'file size' msg
		printf("%s was found in your current directory.\n", buffer);
		memset(msg, 0, sizeof(msg));
		strcat(msg, "1 ");	//will send one if file was found
		FILE *fp;
		fp = fopen(buffer, "rb");
		fseek(fp, 0L, SEEK_END);
		int sz = ftell(fp);
		char size[(int)((ceil(log10(sz))+1)*sizeof(char))];
		sprintf(size, "%d", sz);
		rewind(fp);
		strcat(msg, size);	//will send file size in the same msg

		sleep(1);
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
		memset(msg, 0, sizeof(msg));

		sleep(1);
		//Send 'file name' response
		strcat(msg, buffer);
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
		//printf("Sent name of the file: %s\n", msg);
		memset(msg, 0, sizeof(msg));

		//Send file data
		sendFile(fp, sz, descriptor);
		//fclose(fp);
	}
	
	//Send error message
	else
	{
		printf("%s is not in the current directory.\n", buffer);
		memset(msg, 0, sizeof(msg));
		strcat(msg, "0");
		strcat(msg, buffer);
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
	}

	memset(msg, 0, sizeof(msg));
	memset(buffer, 0 ,sizeof(buffer));
}


int main(int argc, char *argv[])
{
	int sd; /* socket descriptor */
	struct sockaddr_in server_address;
	char bufferO[1000];
	char bufferI[1000];
	char newBuffer[1000];
	int portNumber;
	char serverIP[29];
	int rc = 0;


	if (argc < 3){
		printf ("usage is client <ipaddr> <port>\n");
		exit(1);
	}

	sd = socket(AF_INET, SOCK_STREAM, 0); /* socket descriptor */

	portNumber = strtol(argv[2], NULL, 10); 
	strcpy(serverIP, argv[1]); 

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(portNumber); 
	server_address.sin_addr.s_addr = inet_addr(serverIP);

	if (connect (sd, (struct sockaddr *) &server_address, sizeof (struct sockaddr_in)) < 0)
	{
		close (sd);
		perror ("error connecting stream socket");
		exit (1);
	}


	//Data exchange loop
	while(true)
	{

		//Send a message to the server
		memset(bufferO, 0, 1000);
		memset(bufferI, 0, 1000);
		fgets(bufferO, 1000, stdin);
		//printf("The command entered is: %s\n", bufferO);
		if (send(sd, bufferO, 1000, 0) == -1)
			perror("send");
		//printf ("sent %ld bytes\n", sizeof(bufferO));
		
		//Exit command
		if(strcmp(bufferO, "bye\n") == 0)
			bye(sd);

		//Catalog command
		else if(strncmp(bufferO, "catalog", 7) == 0)
		{
			cat(sd, bufferI);
		}

		//Download command
		else if(strncmp(bufferO, "download ", 9) == 0)
			download(sd, newBuffer); //receive and process message

		else if(strncmp(bufferO, "upload ", 7) == 0)
			upload(sd, bufferO);

		//Receive other responses from the server
		else
			printf("Invalid command.\n\n");
			//receive(sd, bufferI);		
	}
	return 0;
} 
