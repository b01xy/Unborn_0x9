#ifndef UDP_API_H
#define UDP_API_H

#include<stdio.h> //for printf
#include<stdlib.h> //for exit()
#include<stdint.h> //int16_t etc
#include<sys/socket.h> //for socket
#include<sys/types.h> //utile?
#include<arpa/inet.h> //for inet_addr
#include<errno.h> //for error
#include<unistd.h> //for close()
#include<netdb.h> //for gethostbyname
#include<pthread.h> //for thread

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDDR;


//UDP server funtions

int init_UDP_server(SOCKET* server_sock, SOCKADDR_IN *client_sin, socklen_t *csock_size, int Port); //init udp server and wait for ok message to access client socket parameters
// Warning, you must ensure the client will send data at the same time the function is waiting for it or add a while loop to ensure it
int init_UDP_server_no_validation(SOCKET* server_sock, SOCKADDR_IN *client_sin, int Port); //don't wai for message 
void close_UDP_server(SOCKET* sock); //close properly socket


//UDP client functions
void init_UDP_client(SOCKET* client_sock, SOCKADDR_IN *client_sin, char* IP, int PortUDP); //udp client initialisation coherent with init_UPD_server 
void init_UDP_client2(SOCKET* client_sock, SOCKADDR_IN *client_sin, char* IP, int PortUDP); //init udp client with a given IP address
void init_UDP_client3(SOCKET* client_sock, SOCKADDR_IN *client_sin, int PortUDP); //init udp client with the IP address previously sett in client_sin
void close_UDP_client(SOCKET* sock); //close properly socket (same as close_udp_server)


//Data compression/extraction for US card emulation/communication must  be put elsewhere
void compress_data (uint16_t *RAW, uint16_t *comp, unsigned int Nraw); //Nraw must be a multiple of 4
void extract_data (uint16_t *RAW, uint16_t *comp, unsigned int Nraw);

#endif
