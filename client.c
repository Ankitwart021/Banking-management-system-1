#include <stdio.h>      // Import for `printf` & `perror` functions
#include <errno.h>      // Import for `errno` variable
#include <fcntl.h>      // Import for `fcntl` functions
#include <unistd.h>     // Import for `fork`, `fcntl`, `read`, `write`, `lseek, `_exit` functions
#include <sys/types.h>  // Import for `socket`, `bind`, `listen`, `connect`, `fork`, `lseek` functions
#include <sys/socket.h> // Import for `socket`, `bind`, `listen`, `connect` functions
#include <netinet/ip.h> // Import for `sockaddr_in` stucture
#include <string.h>     // Import for string functions
#include <stdlib.h>

void portal_handler(int socketFileDescriptor) {
    char readBuffer[1000], writeBuffer[1000];
    ssize_t readBytes, writeBytes;

    while (1) {
        // Clear buffers for each iteration
        bzero(readBuffer, sizeof(readBuffer));
        bzero(writeBuffer, sizeof(writeBuffer));

        // Read from the server
        readBytes = read(socketFileDescriptor, readBuffer, sizeof(readBuffer));
        if (readBytes == -1) {
            perror("Error while reading from server");
            break;
        } else if (readBytes == 0) {
            printf("Server closed the connection.\n");
            break;
        }

        // Print server's message to the client terminal
        printf("%s\n", readBuffer);

        // Check if server asked to close the connection
        if (strstr(readBuffer, "Logging out") != NULL) {
            break;  // Exit the loop after logging out
        }

        // Get input from the client
        printf("Your input: ");
        scanf("%[^\n]%*c", writeBuffer);

        // Send input to the server
        writeBytes = write(socketFileDescriptor, writeBuffer, strlen(writeBuffer));
        if (writeBytes == -1) {
            perror("Error while writing to server");
            break;
        }
    }
    close(socketFileDescriptor);  // Close the socket once done
}

int main() {
    int socketFileDescriptor, connectStatus;
    struct sockaddr_in serverAddress;

    // Create the socket
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1) {
        perror("Error while creating client socket");
        exit(0);
    }

    // Configure server address
    serverAddress.sin_family = AF_INET; // IPv4
    serverAddress.sin_port = htons(8080); // Port 8080
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Bind to any local address

    // Connect to the server
    connectStatus = connect(socketFileDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectStatus == -1) {
        perror("Error while connecting to the server");
        close(socketFileDescriptor);
        exit(0);
    }

    // Handle the portal logic (interaction between client and server)
    portal_handler(socketFileDescriptor);

    close(socketFileDescriptor);  // Close the socket before exiting
    return 0;
}


