#include"UDP_API.h"

//UDP server functions

int init_UDP_server(SOCKET* server_sock, SOCKADDR_IN *client_sin, socklen_t *csock_size, int Port)
{
	SOCKADDR_IN server={0};

	//Create socket
	(*server_sock)=socket(AF_INET, SOCK_DGRAM,0);
	if ((*server_sock)==INVALID_SOCKET)
	{
		printf("error unsing socket()");
		return 1;
	}

	//Socket info
	server.sin_family=AF_INET;
	server.sin_port=htons(Port);
	server.sin_addr.s_addr=INADDR_ANY; //allow all IP to access to the server

	//Allow other sockets to bind() to this port, use to avoid Address already in use
	int tr=1;
	if (setsockopt((*server_sock),SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) 
	{
		printf("error using setsockopt()");
		return 2;
	}

	//Link communication point
	if (bind((*server_sock), (SOCKADDR *)&server, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		printf("error using bind()");
		return 3;
	}
	
    printf("waiting for \"ok\" to validate server initialisation\n");
	char init_mess[2];
    if((recvfrom((*server_sock), init_mess, 2, 0, (SOCKADDR *)client_sin, csock_size))<0)
    {
        perror("recvfrom()");
        exit(errno);
    }
    
    return 0;
}

int init_UDP_server_no_validation(SOCKET* server_sock, SOCKADDR_IN *client_sin, int Port)
{
	SOCKADDR_IN server={0};

	//Create socket
	(*server_sock)=socket(AF_INET, SOCK_DGRAM,0);
	if ((*server_sock)==INVALID_SOCKET)
	{
		printf("error using socket()");
		return 1;
	}

	//Socket info
	server.sin_family=AF_INET;
	server.sin_port=htons(Port);
	server.sin_addr.s_addr=INADDR_ANY; //allow all IP to access to the server

	//Allow other sockets to bind() to this port, use to avoid Address already in use
	int tr=1;
	if (setsockopt((*server_sock),SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) 
	{
		printf("error using setsockopt()");
		return 2;
	}

	//Link communication point
	if (bind((*server_sock), (SOCKADDR *)&server, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		printf("error using bind()");
		return 3;
	}
	
	printf("UDP server ready\n");
    //server must receive a message to know the adress of the client, it can't send a message before
    //note that listening (recvfrom) is a blocking with flag 0
    return 0;
}

void close_UDP_server(SOCKET *server_socket)
{
	if (shutdown((*server_socket), SHUT_RDWR)!=0)
	{
		perror("shutdown client");
		//exit(1);
	}
	if (close((*server_socket))!=0)
	{
		perror("close");
		//exit(1);
	}
}

//UDP client functions

void init_UDP_client(SOCKET* client_sock, SOCKADDR_IN *client_sin, char* IP, int PortUDP)
{
    *client_sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (*client_sock==INVALID_SOCKET)
    {
        perror("socket() client");
        exit(errno);
    }
    
    client_sin->sin_addr.s_addr=inet_addr("192.168.128.3");
    client_sin->sin_family=AF_INET;
    client_sin->sin_port=htons(PortUDP);
    
    //connection initiallisation message
    char message[2];
    message[0]='o';
    message[1]='k';
    
    int ll=0;
    ssize_t ret=0;
    for (ll=0 ; ll<20 ; ll++)
    {
        ret=sendto(*client_sock, message, 2, 0, (SOCKADDR *)client_sin, sizeof(*client_sin));
        printf("ret = %i\n", ret);
    }
    /*if (sendto(*client_sock, message, 2, 0, (SOCKADDR *)client_sin, sizeof(*client_sin))==SOCKET_ERROR)
    {
        perror("sendto()");
        exit(errno);
    }*/
}

void init_UDP_client2(SOCKET* client_sock, SOCKADDR_IN *client_sin, char* IP, int PortUDP)
{
    *client_sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (*client_sock==INVALID_SOCKET)
    {
        perror("socket() client");
        exit(errno);
    }
    
    //client_sin->sin_addr.s_addr=inet_addr("0.0.0.0");
    //client_sin->sin_addr.s_addr=inet_addr(IP);
    client_sin->sin_addr.s_addr=htonl(INADDR_ANY);
    client_sin->sin_family=AF_INET;
    client_sin->sin_port=htons(PortUDP);
    printf("UDP client ready\n");
}

void init_UDP_client3(SOCKET* client_sock, SOCKADDR_IN *client_sin, int PortUDP)
{
    *client_sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (*client_sock==INVALID_SOCKET)
    {
        perror("socket() client");
        exit(errno);
    }
    
    //client_sin->sin_addr.s_addr=inet_addr("192.168.128.3");
    client_sin->sin_family=AF_INET;
    client_sin->sin_port=htons(PortUDP);
    printf("UDP client ready\n");
}

void close_UDP_client(SOCKET* sock)
{
	if (shutdown((*sock), SHUT_RDWR)!=0)
	{
		perror("shutdown client");
		//exit(1);
	}
	if (close((*sock))!=0)
	{
		perror("close");
		//exit(1);
	}
}



//Data compression/extraction
void compress_data (uint16_t *RAW, uint16_t *comp, unsigned int Nraw) //Nraw must be a multiple of 4
{
    if (Nraw%4!=0) {printf("Warning wrong size of raw data, must be a multiple of 4\n");}
    unsigned int nloop=Nraw/4, i=0;
    
    for (i=0 ; i<nloop ; i++)
    {
        comp[3*i]=(RAW[4*i]<<4)+(RAW[4*i+1]>>8);
        comp[3*i+1]=(RAW[4*i+1]<<8)+(RAW[4*i+2]>>4);
        comp[3*i+2]=(RAW[4*i+2]<<12)+(RAW[4*i+3]);
    }
}

void extract_data (uint16_t *RAW, uint16_t *comp, unsigned int Nraw)
{
    if (Nraw%4!=0) {printf("Warning wrong size of raw data, must be a multiple of 4\n");}
    unsigned int nloop=Nraw/4, i=0;
    
    for (i=0 ; i<nloop ; i++)
    {
        RAW[4*i]=comp[3*i]>>4;
        comp[3*i]-=RAW[4*i]<<4;
        RAW[4*i+1]=(comp[3*i]<<8)+(comp[3*i+1]>>8);
        comp[3*i+1]-=RAW[4*i+1]<<8;
        RAW[4*i+2]=(comp[3*i+1]<<4)+(comp[3*i+2]>>12);
        comp[3*i+2]-=RAW[4*i+2]<<12;
        RAW[4*i+3]=comp[3*i+2];
    }
}
