/* Team Members: Molly Zachlin, Auna Walton
 * NetIDs: mzachlin, awalton3
 *
 * myftpd.cpp - The server side of the TCP file application 
 *
 * */

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <time.h>

#define MAX_SIZE 4096
#define MAX_PENDING 5

using namespace std;

/* Display error messages */
void error(int code) {
    switch (code) {
        case 1:
            cout << "Usage: ./myftpd PORT" << endl;
        break;
        case 2:
            cout << "Unrecognized command." << endl;
        break;
        default:
            cout << "There was an unexpected error." << endl;
        break;
    }
}


/* Execute LS command and return results to client */ 
void LS(int new_sockfd) {
    
    /* Run ls on server and capture output */
    char results[BUFSIZ];
    system("ls -l > lsRes.txt");
    
    /* Return results to client */
    FILE* lsFile = fopen("lsRes.txt", "r");
    fread(results, BUFSIZ, 1, lsFile);

    if (send(new_sockfd, results, strlen(results) + 1, 0) == -1) {  //TODO: fix size in case ls has more than 4096 bytes of output
        perror("Sending results of LS failed.");
        return;
    }  	
}

/* Get the first 10 lines of the specified file */
void HEAD(int new_sockfd, char* file) {
    
    cout << "file is : " << file << endl;
    
    // Check whether file exists
    struct stat s;
    if (stat(file, &s) < 0) {
        perror("File does not exist."); //TODO: return -1 to client
        if (send(new_sockfd, -1, 1, 0) == -1) {  //TODO: fix size in case ls has more than 4096 bytes of output
            perror("Sending HEAD error code failed.");
            return;
        }             
        return;
    }
    
    // Create command
    char head_cmd[MAX_SIZE] = "head ";
    char end_cmd[15] = " > headRes.txt";
    strcat(head_cmd, file);
    strcat(head_cmd, end_cmd);
    
    // Execute command
    system(head_cmd);
    
    // Return results to client 
    char results[BUFSIZ];
    FILE* headFile = fopen("headRes.txt", "r"); //FIXME: notice this code is the same as LS... should we make a function?  or no?
    fread(results, BUFSIZ, 1, headFile);

    if (send(new_sockfd, results, strlen(results) + 1, 0) == -1) {  //TODO: fix size in case ls has more than 4096 bytes of output
        perror("Sending results of HEAD failed.");
        return;
    }  	
}

int main(int argc, char** argv) {

    /* Parse command line arguments */
    if (argc < 2) {
        error(1);
        return 1;
    }

    int port = stoi(argv[1]);

    /* Create address data structure */
    struct sockaddr_in sock;
    // Clear memory location
    bzero((char*) &sock, sizeof(sock));
    // Specify IPv4
    sock.sin_family = AF_INET;
    // Use the default IP address of the server
    sock.sin_addr.s_addr = INADDR_ANY;
    // Convert from host to network byte order
    sock.sin_port = htons(port);

    /* Set up socket */
    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed.");
        return 1;
    }

    /* Set socket options */
    int opt = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(char*)&opt, sizeof(int))) < 0) {
        perror("Setting socket options failed.");
        return 1;
    }

    /* Bind */ 
    if ((bind(sockfd, (struct sockaddr*) &sock, sizeof(sock))) < 0) {
        perror("Bind failed.");
        return 1;
    }

    /* Listen */
    if ((listen(sockfd, MAX_PENDING)) < 0) {
        perror("Listen failed.");
        return 1; 
    } 

    /* Wait for incoming connections */
    int new_sockfd;
    struct sockaddr_in client_sock;
    socklen_t len = sizeof(client_sock);
    while (1) {

        cout << "Waiting for connections on port " << port << endl;
        if ((new_sockfd = accept(sockfd, (struct sockaddr*) &client_sock, &len)) < 0) {
			cout << "exiting " << endl; 
            perror("Accept failed.");
			cout << "exiting " << endl; 
            return 1;
        }

        cout << "Connection established." << endl;

        /* Continue to receive commands */ 
	    char command[MAX_SIZE];
        while (1) {

			if (recv(new_sockfd, command, sizeof(command), 0) == -1) {
            	perror("Error receiving command from client.");   
				return 1; 
       		}

	    	/* Handle commands */ 
			char delim[] = " ";
            if (strcmp(command, "LS") == 0) {
				LS(new_sockfd);
			} 
            else if (strncmp(command, "HEAD", 4) == 0) {
                char* file = strtok(command, delim);
                file = strtok(NULL, delim);
                HEAD(new_sockfd, file);
            }
            else if (strcmp(command, "QUIT") == 0) {
                close(new_sockfd);
                break;
            }
            else {
                error(2);
        	}
    	}
	} 
  
    /* Continuously receive messages */
    //char clientKey[MAX_SIZE];
    /*char buf[MAX_SIZE];

    while (1) {
        cout << "Waiting ..." << endl;
        // Receive client's public key
        if (recvfrom(sockfd, clientKey, sizeof(clientKey), 0, (struct sockaddr*) &client_sock, &len) == -1) {
            cout << "Error receiving public key from client." << endl;   
            return 1;
        } 
        // Generate server public key
        char* servKey = getPubKey();
        // Encrypt and return to client
        char* servEncr = encrypt(servKey, clientKey);
        if (sendto(sockfd, servEncr, strlen(servEncr) + 1, 0, (struct sockaddr*) &client_sock, sizeof(struct sockaddr)) == -1) {
            cout << "Error sending public key to client." << endl;
            return 1;
        }  
        // Receive message
        if (recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*) &client_sock, &len) == -1) {
            cout << "Error receiving message from client." << endl;   
            return 1;
        }
        // Record local timestamp
        time_t timer;
        time(&timer);
        struct tm* stamp = localtime(&timer);
        char* timeStr =  asctime(stamp); 
        // Decrypt message
        char* decryptMsg = decrypt(buf);
        // Print message, time, and checksum
        cout << "******* New Message *******" << endl;
        cout << "Received Time: " << timeStr;
        cout << "Received Message:" << endl;
        cout << decryptMsg << endl << endl;
        //Receive checksum
        struct cksumStruct {
            unsigned long num;
        }; 
        struct cksumStruct cksum;
        if (recvfrom(sockfd, &cksum, sizeof(cksum), 0, (struct sockaddr*) &client_sock, &len) == -1) {
            cout << "Error receiving checksum from client." << endl;
            return 1;
        } 
        // Convert to host byte order
        cksum.num = ntohl(cksum.num);
        cout << "Received Client Checksum: " << cksum.num << endl;
        // Calculate checksum
        unsigned long myCksum = checksum(decryptMsg);
        cout << "Calculated Checksum: " << myCksum << endl << endl;
        if (myCksum != cksum.num) { // Checksums do not match; send error to client
            cout << "Error: Calculated checksum does not match checksum received from client." << endl;
            timeStr = NULL;
        }
        // Send timestamp to client
        if (sendto(sockfd, timeStr, strlen(timeStr) + 1, 0, (struct sockaddr*) &client_sock, sizeof(struct sockaddr)) == -1) {
            cout << "Error sending timestamp to client." << endl;
            return 1;
        }
        // Clear buffer
        bzero((char *)&buf, sizeof(buf));
    }*/

    close(sockfd);
    
    return 0;
}
