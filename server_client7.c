// STUDENT: Matthew Teets
// CLASS: CSCI 3800
// ASSIGNMENT: Lab7

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#define STDIN 0
#define VERSION 3

// Structure that holds info of config.txt file
struct _configure {
    int port_num;
    char ip[14];
};

// Structure that holds info pertaining to the message being sent
struct _msg{
    int version;
    int location;
    int originPort;
    int destPort;
    int hopCount;
    char command[20];
    char msg[100];
    int msg_id;
    int route[3];
};

/*========== Unused structures ==========*/
struct _partnersHost{
    char ip[25];
    int portNumber;
    int location;
    int seqNumSent;
    int ackNumSent;
    int seqNumRec;
    int ackNumRec;
};

struct _partners{
    int maxHosts;
    struct _partnersHost hostInfo[50];
};
/*=======================================*/

/* Function Declarations */
int findCoordinates(int choice, int *row, int *column, int ROWS, int COLUMNS); // Generates coordinates for a given location within a given grid
int distance(int c1, int c2, int r1, int r2); // Uses the euclidean distance formula to calculate the distance between two points
int sendData(char *buffer, int sd, struct sockaddr_in server_address); // This function allows the client to send data to the server
void parseMe(char *line, struct _msg *message); // Parses through the sent message and tokenizes the colon delimited values.

// ====================================================================//
// =============================== MAIN ===============================//
// ====================================================================//

int main(int argc, char * argv[]) {
    
    struct _configure s1[100];
    struct _partners Partners;
    
    // Opens config file
    FILE *f = fopen("config.txt", "r");
    char line[250];
    
    // Error checking if file exists
    if (f == NULL)
    {
        printf("Error: No file \n");
    }
    
    // Reads config file into struct members for each server
    int MAXPARTNERS = 0;                    // Variable for the number of servers being connected
    while (fgets(line, sizeof(line), f))    // returns NULL when no more data
    {
        sscanf(line, "%s %d", s1[MAXPARTNERS].ip, &s1[MAXPARTNERS].port_num);
        MAXPARTNERS++;
    }
    
    fclose(f); // Closes file when we have what we need
    
    /* Variables used throughout the program */
    char buffer[100];       // Variable for the message entered by the user
    char bufferSend[100];   // Variable for the message being sent to the server(s)
    char bufferRecv[100];   // Variable for the message being received from the client(s)
    char bufferACK[100];    // Variable for the ACK message that is sent to the original sender
    char serverIP[29];      // Variable for the IP address of server to be stored
    char *ptr;              // Variable for pointer ptr
    int rc;                 // Varaible for the return code
    int flags = 0;          // Variable recv function
    int messagVersion;      // Variable for version comparison
    
    /* Server/Client information variables */
    struct sockaddr_in from_address;                // Structure variable for the from address of the message
    socklen_t fromLength = sizeof(struct sockaddr); // Sets variable to take the length of socket related parameters
    fromLength = sizeof(struct sockaddr_in);        // Size variable for the length of the from address
    fd_set socketFDS;                               // Set of socket discriptors
    int maxSD;                                      // Tells the OS how many sockets are set
    struct _msg message;                            // Structure variable for the client message
    
    // Checks if number of cmd line arguments is correct
    if (argc < 3) {
        printf("client4 <portnumber> <location>\n");
        exit(1);
    }
    
    int sd;                             // Socket descriptor variable
    struct sockaddr_in server_address;  // Provides address info for current server
    struct sockaddr_in partner_address; // Provides address into for other servers
    
    /* Bind requirements */
    int portNumber = atoi(argv[1]);
    fromLength = sizeof(struct sockaddr_in);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY;
    
    /* Bind actions */
    sd = socket(AF_INET, SOCK_DGRAM, 0);                                        // Creates the socket
    rc = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));  // Binds the socket to the local address info
    
    // Checks if the bind was successful
    if (rc < 0) {
        perror("bind");
        exit(1);
    }
    
    printf("-------------------------------\n");
    
    /* Message Variables */
    int location = atoi(argv[2]);               // Variable stores the users location
    printf("My location: %d\n", location);
    int ROWS, COLUMNS;                          // Variables hold the matrix W x H
    printf("Enter grid size (N M): ");
    scanf("%d %d", &ROWS, &COLUMNS);
    ptr = fgets(buffer, sizeof(buffer), stdin); // Gets the buffer message excluding the newline char
    int originPort = atoi(argv[1]);             // Variable for the origin port number
    int destPort;                               // Variable for the destination port number
    int hopCount = 4;                           // Variable for the hop counter
    int msg_id = 1;                             // Variable for the message ID
    int route[3] = {0,0,0};                     // Variable for the message route
    int countRoute = 0;
    
    // Variables used to obtain/store coordinates of location in the grid
    int choice_r;   // = x coord
    int choice_c;   // = y coord

    //================================= INFINITE LOOP =================================//
    for(;;)
    {
        fflush(stdin); // Used to clear the output buffer
        printf("Enter port number and message plz: \n"); // Prompts user for port and message
        
        // Sets all buffer characters to '\0'
        memset(buffer, '\0', 100);
        memset(bufferSend, '\0', 100);
        memset(bufferRecv, '\0', 100);
        memset(bufferACK, '\0', 100);
        
        FD_ZERO(&socketFDS);        // Initializes set to contain no file descriptors
        FD_SET(sd, &socketFDS);     // Sets the value for sd in the set
        FD_SET(STDIN, &socketFDS);  // Sets the value for STDIN in the set
        
        // Checks descriptor sizes
        if (STDIN > sd)
        {
            maxSD = STDIN;
        }else{
            maxSD = sd;
        }
        
        rc = select (maxSD + 1, &socketFDS, NULL, NULL, NULL); // Sets which file descriptor is ready to be read
        
        /* Received information from the user */
        if (FD_ISSET(STDIN, &socketFDS))
        {
            memset(buffer, '\0', 100);
            scanf("%d", &destPort);                     /* DMO get the port number */
            ptr = fgets(buffer, sizeof(buffer), stdin); /* DMO get the msg */
            ptr = ptr;
            // Creates protocol message in the discussed format
            buffer[strlen(buffer)-1] = '\0';
            sprintf(bufferSend, "%d:INFO:%d:%d:%d:%d:%d:%d:%d:%d:%s",
                    VERSION, location, originPort, destPort, hopCount, msg_id, route[0], route[1], route[2], buffer);
            printf("Sending: '%s'\n\n", buffer);
            
            int temp;
            
            /* Calls 'sendData' function to send message(s) to the server(s) */
            for (int i = 0; i < MAXPARTNERS; i++)
            {
                temp = s1[i].port_num;
                if (temp == portNumber) {
                    continue;
                }
            
                strcpy(serverIP, s1[i].ip);
                partner_address.sin_family = AF_INET;                   // Sets the address family for the transport address
                partner_address.sin_port = htons(s1[i].port_num);       // Indexes to the 'port_num's of the struct array
                partner_address.sin_addr.s_addr = inet_addr(serverIP);  // Indexes to the 'ip's of the struct array
                sendData(bufferSend, sd, partner_address);              // Calls the 'sendData' funtion
            }
        }
    
        /* Received information from the server */
        if (FD_ISSET(sd, &socketFDS))
        {
            rc = recvfrom(sd, bufferRecv, sizeof(bufferRecv), flags, (struct sockaddr *)&from_address, &fromLength);
            parseMe(bufferRecv, &message); // Calls the 'parseMe' function
            
            //================================= LOCATION STUFF =================================//
            
            // Find the coordinates of the user location
            int x1, y1;
            int ret_1 = findCoordinates(location, &choice_r, &choice_c, ROWS, COLUMNS);
            if (-1 == ret_1) // If true then user location is NOT IN GRID
            {
                goto end_msg; // Jumps to end of message
            } else {
                // Stores user coords inside (x1, y1)
                x1 = choice_r;
                y1 = choice_c;
            }
            
            // Find the coordinates of the message location
            int x2, y2;
            int ret_2 = findCoordinates(message.location, &choice_r, &choice_c, ROWS, COLUMNS);
            if (-1 == ret_2) // If true then message location is NOT IN GRID
            {
                goto end_msg; // Jumps to end of message
            } else {
                // Stores message coords inside (x2, y2)
                x2 = choice_r;
                y2 = choice_c;
            }
            
            int distanceVal = distance(x1, x2, y1, y2); // Calculates and displays the distance between the user location and message location
            
            //================================= MESSAGE FORWARDING STUFF =================================//
            
            if (distanceVal > 2) // If the distance between the two coords is greater than 2 OUT OF RANGE
            {
                printf("OUT OF RANGE\n\n");
                goto end_msg; // Avoids printing the data message and goes to the end of the transmission
            }
            
            if(distanceVal <= 2) // If is IN RANGE
            {
                /* Error checking for VERSION */
                sscanf(bufferRecv, "%d", &messagVersion);
                if (messagVersion != VERSION) {
                    printf("\nERROR: Incompatible VERSION!\n");
                    continue;
                }
                
                /* Update the message route tracker */
                
//                printf("\n (%d, %d, %d) \n", originPort, message.destPort, message.destPort);
//                countRoute++;
//                if (countRoute == 1) {
//                    message.route[0] = originPort;
//                }
//                else if (countRoute == 2) {
//                    message.route[1] = message.destPort;
//                }
//                else if (countRoute == 3) {
//                    message.route[2] = message.destPort;
//                }

                if(message.destPort == originPort) {
                    
                    /* Prints and formats all data recieved data */
                    printf("\n************************************\n"
                           "%s MESSAGE RECEIVED : "
                           "\n************************************\n"
                           "version = %d \t\tlocation = %d     "
                           "\noriginPort = %d \tdestPort = %d  "
                           "\ncommand = %s  \t\tmsg_ID = %d      "
                           "\nMessage route = (%d, %d, %d)     "
                           "\nhopCount = %d \t\tmessage = %s   "
                           "\n************************************\n",
                           message.command, message.version, message.location, message.originPort,
                           message.destPort, message.command, message.msg_id, message.route[0],
                           message.route[1], message.route[2] , message.hopCount, message.msg);
                    
                    if (!strcmp(message.command, "ACK")){
                        continue;
                    }
                    
                //=============== ACK message stuff ===============//
                    printf("Sending ACK to sender...\n");
                    
                    /* Creates ACK message */
                    memset(bufferACK, '\0', 100);
                    sprintf(bufferACK, "%d:ACK:%d:%d:%d:%d:%d:%d:%d:%d:%s",
                            VERSION, location, message.destPort, message.originPort, message.hopCount, message.msg_id,
                            message.route[0], message.route[1], message.route[2], message.msg);

//                    printf("message.version = %d\n", message.version);
//                    printf("message.location = %d\n", location);
//                    printf("message.originPort = %d\n", message.originPort);
//                    printf("message.destPort = %d\n", message.destPort);
//                    printf("message.command = %s\n", message.command);
//                    printf("message.msg = %s\n", message.msg);
//                    printf("message.msg = %d\n", message.hopCount);
//                    printf("bufferACK: %s\n\n", bufferACK);
//                    printf("MAXPARTNERS = %d\n", MAXPARTNERS);
//                    printf("portNumber : %d\n", portNumber);

                    /* Calls 'sendData' function to send message(s) to the server(s) */
                    int temp;
                    for (int i = 0; i < MAXPARTNERS; i++)
                    {
                        temp = s1[i].port_num;
                        if (temp == portNumber) {
                            continue;
                        }
                        
                        
                        strcpy(serverIP, s1[i].ip);
                        partner_address.sin_family = AF_INET;                   // Sets the address family for the transport address
                        partner_address.sin_port = htons(s1[i].port_num);       // Indexes to the 'port_num's of the struct array
                        partner_address.sin_addr.s_addr = inet_addr(serverIP);  // Indexes to the 'ip's of the struct array
                        sendData(bufferACK, sd, partner_address);               // Calls the 'sendData' funtion
                    }
                    
                /*================== Forwarding Stuff ==================*/
                    
                } else if (message.originPort != originPort) {
                    message.hopCount--;
                    message.location = location;
                    
                    if (message.hopCount > 0)
                    {
                            /* Loop and send to each server */
                        sprintf(bufferSend, "%d:INFO:%d:%d:%d:%d:%s",
                                VERSION, location, message.originPort, message.destPort, message.hopCount, message.msg);
                        
                        printf("************************************\n");
                        printf("Forwarding message to next gnode --->\n");
                        printf("Sending: '%s'", bufferSend);
                        printf("\nHop Count: %d", message.hopCount);
                        printf("\n************************************\n\n");

                        for (int i = 0; i < MAXPARTNERS; i++)
                        {
                            portNumber = s1[i].port_num;
                            strcpy(serverIP, s1[i].ip);
                            partner_address.sin_family = AF_INET;
                            partner_address.sin_port = htons(portNumber);
                            partner_address.sin_addr.s_addr = inet_addr(serverIP);
                            sendData(bufferSend, sd, partner_address);
                        }
                    } else {
                        printf("************************************\n");
                        printf("Hop Count: %d\n", message.hopCount);
                        printf("Ran out of hops!\n");
                        printf("************************************\n\n");
                    }
                }
            }
            
        // goto label to jump to end of the message
         end_msg:
             printf("\n");
        }
    }
    close(sd); // Closes the client socket
    return 0;
}

// ===============================================================================//
// ================================== FUNCTIONS ==================================//
// ===============================================================================//

// This function allows the client to send data to the server
int sendData(char *buffer, int sd, struct sockaddr_in server_address) {
    /*
       All information being sent out to the server :
            sd                                   ->  socket descriptor
            buffer                               ->  data being sent
            strlen(buffer)                       ->  how many bytes of data being sent
            0                                    ->  flags
            (struct sockaddr *) &server_address  ->  TO: address of the server
            sizeof(server_address)               ->  size of the data structure being sent
     */
    
    int rc = 0;
    rc = sendto(sd, buffer, strlen(buffer), 0, (struct sockaddr *) &server_address, sizeof(server_address));
    // printf ("DMO: sending '%s' to socket %d, address %s\n", buffer, sd, inet_ntoa(server_address.sin_addr));
    // Checks if sendto was successfully filled
    if(rc <= 0)
    {
        printf("ERROR: No bytes were sent/received... \n\n");
        exit(1);
    }
    return(0);
}

// =========================================================================//

// Parses through the sent message and tokenizes the colon delimited values.
void parseMe(char *line, struct _msg *message){
    /* sprintf(bufferSend, "%d:INFO:%d:%d:%d:%d:%d:<%d:%d:%d>:%s",
       VERSION, location, originPort, destPort, hopCount, msg_id, route[0], route[1], route[2], buffer); */
    
    int version;
    char command[20];
    int location;
    int originPort;
    int destPort;
    int hopCount;
    char *ptr;
    char msg[100];
    int msg_id;
    int route[3];

    version = atoi(strtok(line, ":"));
    
    ptr = strtok(NULL, ":");
    sprintf(command, "%s", ptr);
    
    ptr = strtok(NULL, ":");
    location = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    originPort = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    destPort = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    hopCount = atoi(ptr);
    
    /* msg_id and routing numbers */
    
    ptr = strtok(NULL, ":");
    msg_id = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    route[0] = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    route[1] = atoi(ptr);
    
    ptr = strtok(NULL, ":");
    route[2] = atoi(ptr);
    
    /* + + + + + + + + + + + + */
    
    ptr = strtok(NULL, ":");
    sprintf(msg, "%s", ptr);
    
    /* Push all info into the message struct */
    message -> version = version;
    message -> location = location;
    message -> originPort = originPort;
    message -> destPort = destPort;
    message -> hopCount = hopCount;
    /* msg_id and routing numbers */
    message -> msg_id = msg_id;
    message -> route[0] = route[0];
    message -> route[1] = route[1];
    message -> route[2] = route[2];
    sprintf(message -> command, "%s", command);
    sprintf(message -> msg, "%s", msg);
}

//=========================================================================//

// Generates coordinates for a given location within a given grid
int findCoordinates(int choice, int *row, int *column, int ROWS, int COLUMNS) {
  /*
     - ROWS and COLUMNS are the size of the grid
     - row and column are returned as location of the ‘choice’
     - choice is the cell you are in
   */
    
    *row = (choice - 1) / COLUMNS + 1;
    *column = (choice - 1) % COLUMNS + 1;
    
    if (*row > ROWS) {
        printf ("\nNOT IN GRID\n\n");
        return(-1);
    }else{
        //printf ("Your row/column: ( %d / %d ) \n",*row, *column);
    }

    return(1);
    
}

//=========================================================================//

// Uses the euclidean distance formula to calculate the distance between two points
int distance(int c1, int c2, int r1, int r2) {
    return (int)sqrt(pow(c2 - c1, 2) + pow(r2 - r1, 2));
}

//=========================================================================//


    

