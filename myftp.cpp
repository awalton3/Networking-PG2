/* Molly Zachlin, Auna Walton 
 * mzachlin, awalton3
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

#define MAX_SIZE 4096


using namespace std;

/* Display error messages */
void error(int code) {
    switch (code) {
        case 1:
            cout << "Usage: ./myftp HOSTNAME PORT" << endl;
        break;
		case 2: 
			cout << "Available Options: \n DN: Download \n UP: Upload \n HEAD: Head of File \n RM: Remove File \n LS: List\n" 			  "MKDIR: Make Directory\n RMDIR: Remove Directory\n CD: Change Directory\n QUIT: Quit\n"; 
		break; 		
		default:
            cout << "There was an unexpected error." << endl;
        break;
    }
}

/* List files on server */ 
void LS(int sockfd, struct sockaddr* sock) {

	const char* command = "LS"; 

	if (send(sockfd, command, strlen(command) + 1, 0) == -1) {
        perror("Error sending command to server."); 
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
	while (1) {

		cout << "> "; 
		cin >> command; 

		if (command == "LS")
			LS(sockfd, (struct sockaddr*)&sock);  
		else 
			error(2); 
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
