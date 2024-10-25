#include <stdio.h>
#include <errno.h>

#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/ip.h>

#include<string.h>
#include<stdbool.h>
#include<stdlib.h>

#include "functions/customer_portal.h"
#include "functions/employee_portal.c"
#include "functions/manager_portal.h"
#include "functions/admin_portal.h"

void portal_handler(int connectionFileDescriptor){
	printf("Connection is made\n");
	char readBuffer[1000], writeBuffer[1000];
	ssize_t readBytes, writeBytes;
	int choice;
	writeBytes = write(connectionFileDescriptor, "*********** BANK PORTAL ***********\nPlease specify by pressing:\n1 for customer login\n2 for employee login\n3 for manager login\n4 for admin login\nPress any other number to exit\n", strlen("*********** BANK PORTAL ***********\nPlease specify by pressing:\n1 for customer login\n2 for employee login\n3 for manager login\nPress any other number to exit\n"));
	if(writeBytes == -1){
		perror("Error while sending data to the user");
	}
	else{
		bzero(readBuffer, sizeof(readBuffer));
		readBytes = read(connectionFileDescriptor, readBuffer, sizeof("*********** BANK ACADEMIA PORTAL ***********\nPlease specify by pressing:\n1 for customer login\n2 for employee login\n3 for manager login\n4 for admin login\nPress any other number to exit\n"));
		if(readBytes == -1){
			perror("Error while reading from client");
		}
		else if(readBytes == 0){
			printf("No data was sent to the server\n");
		}
		else{
			choice = atoi(readBuffer);
			switch(choice){
				case 1:
					customer_portal(connectionFileDescriptor);
					break;
				case 2:
				        employee_portal(connectionFileDescriptor);
				        break;
				case 3:
				        manager_portal(connectionFileDescriptor);
						break;
				case 4:
				        admin_portal(connectionFileDescriptor);
						break;
				default:
					break;
				}
		   }
	}
	printf("Closing the connection to server\n");
}
void main(){
    int socketFileDescriptor, socketBindStatus, socketListenStatus, connectionFileDescriptor;
    struct sockaddr_in serverAddress, clientAddress;

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1)
    {
        perror("Error while creating server socket!");
        _exit(0);
    }

    serverAddress.sin_family = AF_INET;                // IPv4
    serverAddress.sin_port = htons(8080);              // Server will listen to port 8080
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Binds the socket to all interfaces

    socketBindStatus = bind(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (socketBindStatus == -1)
    {
        perror("Error while binding to server socket!");
        exit(0);
    }

    socketListenStatus = listen(socketFileDescriptor, 20);
    if (socketListenStatus == -1)
    {
        perror("Error while listening for connections on the server socket!");
        close(socketFileDescriptor);
        exit(0);
    }

    int clientSize;
    while (1)
    {
        clientSize = (int)sizeof(clientAddress);
        connectionFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *)&clientAddress, &clientSize);
        if (connectionFileDescriptor == -1){
		perror("Error while connecting to client!");
        	close(socketFileDescriptor);
		exit(1);
        }
	else{
		if(!fork()){ // child process will handle this client socket
			portal_handler(connectionFileDescriptor);
			close(connectionFileDescriptor);
			exit(0);
		}
		
	}
    }
    close(socketFileDescriptor);
    exit(0);
}
