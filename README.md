# Lab 7
#### Name: Matthew Teets
#### Date: 4 / 5 / 22
#### Class: CSCI 3800

#

### **Description:**
This program allows the server to send ACK messages to the client when the message has reched its destination. Sequence tracking parameters have been added to the protocol message to allow both the sender and receiver to see the path that the message has traveled via port numbers. The port number and location are command line arguments that must be entered when running the server programs (example down below). Once the program is running the user will enter the grid size, and a destination port number to receive the message. The message will only be displayed when it arrives at that destination port number. The message has a limited number of hops(4) to get to the destination. Once the amount of hops has been exhausted, the message is dropped. If the message is **OUT OF RANGE** or **NOT IN GRID** then the program drops the message and restarts. 
- The client/server program can run on any gnode. 
- When running more than one it is recommended to run them on different gnodes. 
- The config.txt contains the IP addresses of 4 gnode servers and 4 port numbers to try out. 

#

### **The program:**
- server_client7.c
  - Promts user for grid size ```"N M"```
  - Then, prompts user for destination port number
  - Reads the config file into a struct
  - Creates the socket/address info using the struct
  - Creates and binds a DGRAM socket to the server address
  - Promts user for a message to send to the server
  - Calculates the distance between the location of the sender and receiver of the message
  - Displays if message sent is **IN RANGE**, **OUT OF RANGE**, or **NOT IN GRID**
  - Checks if the message has arrived at the destination port number
    - if not: forwads message until either 1) it arrives at the destination, or 2) runs out of hops and drops message 
  - Messages from the client are processed, formatted, and printed to the terminal
  - Loops until the program is manually terminated using ```Control + C```

#

### **How to run:**
- Connect to CSE-grid
  - Example: ssh username@csegrid.ucdenver.pvt
  - Connect your terminal window(s) to this Linux server
- cd to the directory containing the c program, makefile, and config.txt

**Terminal window used to run server_client6.c**
```
  $ ssh csci-gnode-NUM   /* Use this command to get each terminal to the correct gnode number (i.e., csci-gnode-01) */
  $ make -f Makefile     /* Compiles the c programs */
  $ ./client7 [SERVER-PORT-NUMBER] [LOCATION]    /* Command to run the executable */
```
**Example:**
```
$ ./client7 1101 3          
$ ------------------------            
$ My location: 3       
$ Enter grid size (N M): 5 5 /* Initializes a 5x5 grid */
$ Enter a destination port: 1104 /* Sets destination port number to receive message */
$         
$ (prints results...)
```
(Repeat this process with different port numbers and locations to connect more servers)
