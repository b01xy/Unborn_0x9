#ifndef TCP_API_H
#define TCP_API_H

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

#include"settings.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDDR;
typedef struct client client; //structure that contained client informations
typedef struct server_info server_info; //structure need to make thread with server routinr

//Structure Management
void init_struct_client(client* client_list,unsigned int Nmax); //initialise client structure
void clear_struct_client(client* client_list); //free malloc on struct client
void add_client(client* client_list, Settings *lSettings, SOCKET sock_server); //TCP server is initialised it add new client informations in structure client
void clear_client(client* client_list,unsigned int id); //clear client with id_client==id form structure client and reorganize the structure

//Conversion function
int int_converter(char x); //convert char to int

//TCP server functions
int init_TCP_server(SOCKET* sock, int Port, client* client_list,unsigned int MaxClient); //initialise TCP server
int init_TCP_server_simple(SOCKET *server_socket, SOCKET *client_socket, SOCKADDR_IN *client_sin, int PORT); //initialise TCP server without client structure
void close_TCP_server(SOCKET* sock, client* client_list); //close client connexion and TCP server (for server)
void close_TCP_server_simple(SOCKET *sock, SOCKET *client_sock); //close TCP server without client structure
int send_TCP_server(client* client_list, char* buffer, int buff_length, int target); // send buffer of size buff_length to client with id target, if target<0 buffer is sent to all clients for server
int receive_TCP_server(client* client_list, char* buffer, int buff_length, int target); //receive buffer of size buff_length from client with id target for server
int send_int16_TCP_server(client* client_list, int16_t *buffer, int buff_length, int target); //same as send_TCP_server but the variable sent are coded on 2 bytes (16 bits int) instead only one use with receive_int16_TCP_client
int receive_int16_TCP_server(client* client_list, uint16_t* buffer, int buff_length, int target);
//can be adapt for each type, e.g. int32, float, double


//Possible thread for managing server
//Comment, uncomment in .h and .c file to use, may manage this with #if
/*
pthread_t TCP_server_thread;
void *TCP_server_routine(void* p_data); //server routine function for thread, server turn in parallel to main
void launch_server(SOCKET* sock, client* client_list, Settings *nSettings); //function that launch the server in parallel to main
pthread_t Settings_server_routine_thread;
void *Settings_server_routine(void *p_data);
void launch_settings_comm(SOCKET *sock, client* client_list, Settings *nSettings);
*/

//TCP client function
int init_TCP_client(SOCKET* sock, const char* IP, int Port); //initialise TCP client
void close_TCP_client(SOCKET* sock); //close client connexion (for client)
void send_TCP_client(SOCKET* sock, char* buffer, int buff_length); //send buffer of size buff_length to server for client
int receive_TCP_client(SOCKET* sock, char* buffer, int buff_length); //receive buffer of size buff from server for client
int peek_TCP_client(SOCKET* sock, int data_size); //read the first data of the buffer
void send_int16_TCP_client(SOCKET* sock, int16_t *buffer, int buff_length); //same as send but for int16_t buffer
int receive_int16_TCP_client(SOCKET* sock, int16_t * buffer, int buff_length); //receive 16 bits buffer of size buff_length from server, use with send int16_TCP_server


//struct client use by a server to manage multiple client 
struct client
{
	unsigned int Nmax;
	unsigned int NbClient;
    unsigned int* id_client;
	SOCKET* sock_client;
	SOCKADDR_IN* sin_client;
};

struct server_info
{
	SOCKET sock;
	client* client_list;
    Settings *RP_settings;
};

#endif
