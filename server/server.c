#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

//Bye function
void bye(int descriptor)
{
	printf("Client closed connection.\n");
	close (descriptor);
	exit (1);
}

//Catalog function
void catalog(int descriptor)
{
	FILE *cmd;
	int status;
	char path[20];
	char result[1000];

	cmd = popen("ls *", "r");
	
	if (cmd == NULL)
		printf("Error occured when opening command...");
	memset(path, 0, sizeof(path));
	memset(result, 0, sizeof(result));
	strcat(result, "\nCatalog:\n");

	while (fgets(path, sizeof(path), cmd) != NULL)
	{
		strcat(result, path);
	}

	//Send the catalog back to client
	if(send(descriptor, result, sizeof(result), 0) == -1)
	    perror("send");
	status = pclose(cmd);
	
	if (status == -1)
	    printf("Error reported by pclose");

	printf("%s", result);
	memset(path, 0, sizeof(path));
	memset(result, 0, sizeof(result));
	printf("Sent catalog successfully...\n");
}

//Send file function
void sendFile(FILE *filePtr, int fileLen, int descriptor)
{
	char *buffer;
	buffer = (char *) malloc(fileLen);
	char symbol;

	memset(buffer, 0, sizeof(buffer));
	
	printf("The file length is %d.\n", fileLen);

	// DMO not optimal, but let's try to see what it does
	int i;
	for (i=0; i<fileLen; i=i+1000){
	  int rc;
	  rc = fread(buffer, 1, 1000, filePtr);		//total number of elements successfully read
	  rc = send(descriptor, buffer, rc, 0);		//number of bytes sent
	  // read in a 1000 bytes and send them
	}

	printf("File sent successfully.\n");

	//Free the memory
	memset(buffer, 0, sizeof(buffer));
	free(buffer); 
	fclose(filePtr);
}

//Download function
void download(char *buffer, int descriptor)
{
	strtok(buffer, " ");
	const char *filename = strtok(NULL, "\n");
	printf("The filename is %s.\n", filename);

	char msg[1000];

	//Send file
	if (access(("./%s",filename), F_OK) == 0)
	{
		//Send 'file found' and 'file size' msg
		printf("%s was found in the current directory.\n", filename);
		memset(msg, 0, sizeof(msg));
		strcat(msg, "1 ");	//will send one if file was found
		FILE *fp;
		fp = fopen(filename, "rb");
		fseek(fp, 0L, SEEK_END);
		int sz = ftell(fp);
		char size[(int)((ceil(log10(sz))+1)*sizeof(char))];
		sprintf(size, "%d", sz);
		rewind(fp);
		strcat(msg, size);	//will send file size in the same msg
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
		printf("Sent 'found' value and size of the file: %s\n", msg);
		memset(msg, 0, sizeof(msg));

		//Send 'file name' response
		strcat(msg, filename);
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
		printf("Sent name of the file: %s\n", msg);
		memset(msg, 0, sizeof(msg));

		//Send file data
		sendFile(fp, sz, descriptor);
		//fclose(fp);
	}
	
	//Send error message
	else
	{
		printf("%s is not in the current directory\n.", filename);
		memset(msg, 0, sizeof(msg));
		strcat(msg, "0");
		//strcat(msg, filename);
		if(send(descriptor, msg, sizeof(msg), 0) == -1)
			perror("send");
	}

	memset(msg, 0, sizeof(msg));
	memset(buffer, 0 ,sizeof(buffer));
}

//Upload function
void upload(char *buffer, int descriptor)
{
	printf("I got here 1.\n");
	//Get file name
	strtok(buffer, " ");
	const char *filename = strtok(NULL, "\n");
	char temp[1000];
	memset(temp, 0, sizeof(temp));
	printf("I got here 2.\n");
	if(filename == NULL)
	{
		printf("ERROR. Missing file name argument.\n\n");
		return;
	}
	printf("I got here 3.\n");

	strcpy(temp, filename);
	buffer = temp;								//buffer now holds the file name
	printf("The filename is %s.\n", buffer);

	//Check if file is in directory
	if (access(("./%s", buffer), F_OK) == 0)
	{
		memset(buffer, 0, sizeof(buffer));
		strcat(buffer, "1");
		if(send(descriptor, buffer, sizeof(buffer), 0) == -1)
			perror("send");
	}
	else
		if(send(descriptor, buffer, sizeof(buffer), 0) == -1)
			perror("send");
	memset(buffer, 0, sizeof(buffer));

	//Receive info about the file
	if(recv(descriptor, buffer, 1000, 0) == -1)
		perror("recv");
	else
		printf("Received initial message: %s \n", buffer);
	
	//File was not found
	if(strncmp(buffer, "0", 1) == 0)
		printf("The file was not found.\n\n");
	//File was found
	else if(strncmp(buffer, "1 ", 2) == 0)
	{
		//File size
		strtok(buffer, " ");
		char *size = strtok(NULL, "\n");
		int x;
		sscanf(size, "%d", &x);
		memset(buffer, 0, 1000);

		//File name
		if(recv(descriptor, buffer, 1000, 0) == -1)
			perror("recv");
		printf("\nReceived file name: %s\n", buffer);
		FILE *fp = fopen(buffer, "wb");

		//Calculate number and size of messages
		char msg[1000];
		int quotient = x / sizeof(msg); 	
		int remainder = x % sizeof(msg);	
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
			  // DMO	fwrite(msg, sizeof(msg), 1, fp);
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
		memset(buffer, 0, 1000);
		memset(msg, 0, sizeof(msg));
		memset(last, 0, remainder);
		printf("Received the entire file successfully.\n\n");
	}
}

int main()
{
	int sd; /* socket descriptor*/
	int connected_sd; /* connected socket descriptor */
	int rc; /* return code from recvfrom */
	struct sockaddr_in server_address;
	struct sockaddr_in from_address;
	char buffer[1000];
	int flags = 0;
	bool client;
	socklen_t fromLength = sizeof(struct sockaddr);

	sd = socket (AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET; /* server address family */
	server_address.sin_port = htons(24000); /* server address port, htons converts an IP port number into network byte order */
	server_address.sin_addr.s_addr = INADDR_ANY; /* address I want to bind to - any address on this machine (from include files) */

	bind (sd, (struct sockaddr *)&server_address, sizeof(server_address)); /* bind the address to a socket descriptor, sets up the server to listen */
	listen (sd, 5); /* how many people are "on hold", take socket descriptor and number of people */

	while(true)
	{
		connected_sd = accept (sd, (struct sockaddr *) &from_address, &fromLength);
		client = true;

		//Data exchange loop
		while (client)
		{
			//Receive commands
			memset(buffer, 0, sizeof(buffer));
			if (recv(connected_sd, buffer, sizeof(buffer), 0) == -1)
			{
				perror("recv");
				client = false;
			}
			printf("%s", buffer);

			//bye command
			if(strcmp(buffer, "bye\n") == 0)
				client = false;

			//catalog command
			else if (strcmp(buffer, "catalog\n") == 0)
			{	
				catalog(connected_sd);
			}
			//download command
			else if(strncmp(buffer, "download ", 9) == 0)
				download(buffer, connected_sd);

			//upload command
			else if(strncmp(buffer, "upload ", 7) == 0)
				upload(buffer, connected_sd);
		}
	}
}
