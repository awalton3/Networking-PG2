/* Team members: Molly Zachlin, Auna Walton 
 * NetIDs: mzachlin, awalton3
 *
 * myftp.cpp - The client side of the TCP file application 
 *
 * */ 

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_SIZE 4096
using namespace std;

typedef struct info_struct {
	short int fn_size;  
	int f_size;
    short int status; 
} info_struct;

/* Display error messages */
void error(int code) {
    switch (code) {
        case 1:
            cout << "Usage: ./myftp HOSTNAME PORT" << endl;
        break;
		case 2: 
			cout << "Available Options: \n DN: Download \n UP: Upload \n HEAD: Head of File \n RM: Remove File \n LS: List\n" 			             " MKDIR: Make Directory\n RMDIR: Remove Directory\n CD: Change Directory\n QUIT: Quit\n"; 
		break; 		
		default:
            cout << "There was an unexpected error." << endl;
        break;
    }
}

void sendcomm(int sockfd, string command) {
	char commandF[BUFSIZ]; 
	strcpy(commandF, command.c_str());  

	if (send(sockfd, commandF, strlen(commandF) + 1, 0) == -1) {
        perror("Error sending command to server."); 
        return;
    }
}

/* List files on server */ 
void LS(int sockfd) {
    char results[BUFSIZ];
    if (recv(sockfd, results, sizeof(results), 0) == -1) {
        perror("Failed to receive results of LS from server.");
        return;
    }
	cout << results << endl; 
}

/* Get the first 10 lines of the specified file from the server */
void HEAD(int sockfd) { 
    //TODO: send length of filename (parse command on this end?) or you can do strlen
    //of command - the amount for HEAD 
    char results[BUFSIZ];
    if (recv(sockfd, results, sizeof(results), 0) == -1) {
        perror("Failed to receive results of HEAD from server.");
        return;
    }
	cout << results << endl; 
}

/* Change directories on the server */
void CD(int sockfd) {
    
}

/* Download a file from the server */ 
void DN(int sockfd, char* filename) {

	cout << "In DN on client side \n"; 
    cout << "Filename on client side: " << filename << endl;

	// Get filename size
	short int fn_size = strlen(filename) + 1;

	cout << "Fn_size: " << fn_size << endl; 

   	// Send filename size to server 
	info_struct info; 
	info.fn_size = htons(fn_size); 

	cout << "Computed hton: " << info.fn_size << endl; 

	if (send(sockfd, &info, sizeof(info_struct), 0) == -1) {
        perror("Error sending filename size to server."); 
        return;
    }

	cout << "Sent filename size to server \n"; 

	// Send filename to server 
	if (send(sockfd, filename, fn_size, 0) == -1) {
        perror("Error sending filename to server."); 
        return;
	}
	cout << "Sent filename to server \n"; 

	// Receive md5sum hash from server 
	char hash[BUFSIZ];
    if (recv(sockfd, hash, sizeof(hash), 0) == -1) {
        perror("Failed to receive hash from server");
        return;
    } 
	cout << "Received hash: " << hash << endl; 

	// Receive filesize from server 
    if (recv(sockfd, &info, sizeof(info), 0) == -1) {
        perror("Failed to receive filesize from server.");
        return;
    }
	info.f_size = ntohl(info.f_size); 
	cout << "Received filesize (actual): " << info.f_size << endl;  

	// Clear existing local copy of file
    char clr_cmd[MAX_SIZE] = "> ";
    strcat(clr_cmd, filename);
    system(clr_cmd);
    // Receive file in 4096 chunks 
	int nread = 0; 
    while (nread < info.f_size) {
		char chunk[MAX_SIZE]; 
        bzero((char*) &chunk, sizeof(chunk)); // Clear memory
        FILE* new_file = fopen(filename, "a");  // File on client side to append text to
		if (recv(sockfd, chunk, sizeof(chunk), 0) == -1) {
        	perror("Failed to receive filesize from server.");
        	return; 
    	}
        // Append chunk to local copy of the file
        fputs(chunk, new_file);
	    nread = nread + strlen(chunk) + 1;
		cout << "**** nread: " << nread << endl; 
        cout << chunk << endl << endl;
        fclose(new_file);  //FIXME: move this and opening file outside of while loop
	} 

	cout << "##### Total: " << nread << endl; 
}

/* Create a directory on the server */ 
void MKDIR(int sockfd, char* dirname) {
    // Get dirname size
	short int dn_size = strlen(dirname) + 1;

	cout << "dn_size: " << dn_size << endl; 

   	// Send dirname size to server 
	info_struct info; 
	info.fn_size = htons(dn_size); 
    if (send(sockfd, &info, sizeof(info_struct), 0) == -1) {
        perror("Error sending dirname size to server."); 
        return;
    }

    // Send filename to server 
	if (send(sockfd, dirname, dn_size, 0) == -1) {
        perror("Error sending dirname to server."); 
        return;
	}
	cout << "Sent dirname to server \n"; 

    // Receive MKDIR status from server
    info_struct s; 
    if (recv(sockfd, &s, sizeof(s), 0) == -1) {
        perror("Failed to receive MKDIR status from server.");
        return;
    }
    
    // Display status
    cout << "raw: " << s.status << endl;
    s.status = ntohl(s.status);
    cout << "conv. " << s.status << endl;
    switch (s.status) {
        case 1:
            cout << "The directory was successfully made" << endl;
            break;
        case -1:
            cout << "Error in making directory" << endl;
            break;
        case -2:
            cout << "The directory already exists on server" << endl;
            break;
        default:
            cout << "An unknown error occurred" << endl;
            break;
    } 
}

int main(int argc, char** argv) {

    /* Parse command line arguments */
    if (argc < 3) { // Not enough input args
        error(1); 
        return 1;
    }

    const char* host = argv[1];
    int port = stoi(argv[2]);
    struct stat statbuf;
    string msg;
    
    /* Translate host name into peer's IP address */
    struct hostent* hostIP = gethostbyname(host);
    if (!hostIP) {
        cout << "Error: Unknown host." << endl;
        return 1;
    }

    /* Create address data structure */
    struct sockaddr_in sock;
    // Clear memory location
    bzero((char*) &sock, sizeof(sock));
    // Specify IPv4
    sock.sin_family = AF_INET;
    // Copy host address into sockaddr
    bcopy(hostIP->h_addr, (char*) &sock.sin_addr, hostIP->h_length);
    // Convert from host to network byte order
    sock.sin_port = htons(port);

    /* Create the socket */
    int sockfd;
    //socklen_t len = sizeof(sock);
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket.");
        return 1;
    }

	cout << "Connecting to " << host << " on port " << port << endl; 

    /* Connect */
    if (connect(sockfd, (struct sockaddr*)&sock, sizeof(sock)) < 0) {
        perror("Error connecting.");
        return 1;
    }

	cout << "Connection established" << endl; 

	/* Wait for user input */ 
	string command;
   	string additional; 	
	while (1) {

		cout << "> "; 
		getline(cin, command);
		sendcomm(sockfd, command); 


		// Parse input
		char command_cstr[BUFSIZ]; 
		strcpy(command_cstr, command.c_str()); 
		char* token = strtok(command_cstr, " ");		
		token = strtok(NULL, " "); 
		
        // Parse input commands 
		if (command == "LS") {
			LS(sockfd);  
        } 
        else if (command.rfind("HEAD", 0) == 0) {  // Checks if the command starts with HEAD
            HEAD(sockfd);
        }
        else if (command.rfind("CD", 0) == 0) {
            CD(sockfd);
        }
		else if (command.rfind("DN", 0) == 0) {
			DN(sockfd, token); 	
        }
        else if (command.rfind("MKDIR", 0) == 0) {
            MKDIR(sockfd, token);
        }
        else if (command == "QUIT") {
            close(sockfd);
            break;
        } 
        else {
			error(2); 
        }
	}


    /* Generate a public encryption key */
    //char* pubKey = getPubKey();

    /* Send key to server */
    /*if (sendto(sockfd, pubKey, strlen(pubKey) + 1, 0, (struct sockaddr*) &sock, sizeof(struct sockaddr)) == -1) {
        perror("Error sending key to server.");
        return 1;
    }*/

    /* Receive server's key */
    /*char servKey[MAX_SIZE]; 
    socklen_t len = sizeof(sock);
    if (recvfrom(sockfd, servKey, sizeof(servKey), 0, (struct sockaddr*) &sock, &len) == -1) {
        cout << "Error receiving server's key." << endl;
        return 1;
    }     
    // Decrypt the key
    char* decryptKey = decrypt(servKey);*/
    
    /* Compute checksum */
    /*char msgArr[MAX_SIZE];
    strcpy(msgArr, msg.c_str());
    unsigned long cksum = checksum(msgArr);
    cout << "Checksum sent: " << cksum << endl;*/

    /* Encrypt message with server's key */
    //char* encryptMsg = encrypt(msgArr, decryptKey);

    /* Send the message */
    /*if (sendto(sockfd, encryptMsg, strlen(encryptMsg) + 1, 0, (struct sockaddr*) &sock, sizeof(struct sockaddr)) == -1) {
        cout << "Error sending message to server." << endl;
        return 1;
    }*/
    
    /* Send the checksum */ 
    /*struct cksumStruct {
        unsigned long num; 
    };
    struct cksumStruct ckSend; 
    ckSend.num = htonl(cksum);
    if (sendto(sockfd, &ckSend, sizeof(ckSend), 0, (struct sockaddr*) &sock, sizeof(struct sockaddr)) == -1) {
        cout << "Error sending checksum to server." << endl;
        return 1;
    }
    // Record sent time
    struct timeval sentTime;
    if (gettimeofday(&sentTime, NULL) < 0) {
        cout << "Failed to get time of day." << endl;
        return 1;   
    } */
 
    /* Receive timestamp from server */
    /*char timeStr[MAX_SIZE];
    if (recvfrom(sockfd, timeStr, sizeof(timeStr), 0, (struct sockaddr*) &sock, &len) == -1) {
        cout << "Error receiving timestamp from server." << endl;
        return 1;
    }*/
    // Record receive time
    /*struct timeval recvTime;
    if (gettimeofday(&recvTime, NULL) < 0) {
        cout << "Failed to get time of day." << endl; 
        return 1;      
    }*/

    /* Display */
    /*if (timeStr == NULL) {
        cout << "Invalid timestamp received from server." << endl;
        return 1;
    }
    cout << "Server has successfully received the message at:" << endl;
    cout << timeStr;
    // Calculate and print RTT
    time_t rtt = recvTime.tv_usec - sentTime.tv_usec;
    cout << "RTT: " << rtt << "us" << endl;*/

    return 0;
}
