#include"TCP_API.h"
 
void init_struct_client(client* client_list, unsigned int Nmax)
{
	client_list->Nmax=Nmax;
	client_list->NbClient=0;
    client_list->sock_client=(SOCKET *)calloc(Nmax, sizeof(SOCKET));
    client_list->sin_client = (SOCKADDR_IN *)calloc(Nmax, sizeof(SOCKADDR_IN));
    client_list->id_client = (unsigned int *)calloc(Nmax, sizeof(unsigned int));
}

void clear_struct_client(client* client_list)
{
    if (client_list->sock_client != NULL)
    {
	    free(client_list->sock_client);
        client_list->sock_client = NULL;
    }
    if (client_list->sin_client != NULL)
    {
	    free(client_list->sin_client);
        client_list->sin_client = NULL;
    }
    if (client_list->id_client != NULL)
    {
	    free(client_list->id_client);
        client_list->id_client = NULL;
    }
}

void add_client(client* client_list, Settings *lsettings, SOCKET sock_server)
{
        printf("start add client function\n");
		unsigned int socklen=sizeof(SOCKADDR_IN);
		SOCKET tmp;
		//Settings_ID lID = na, nID = settings_size;
        //uint8_t *buffer = NULL;
        int buffer_size = 2*settings_size;
        //buffer = (uint8_t *)calloc(2*buffer_size,sizeof(uint8_t));
        uint8_t buffer[buffer_size+1];
        for (int i=0 ; i<buffer_size/2 ; i++)
        {
            printf("i loop %i on %i\n", i, settings_size);
            buffer[2*i] = i;
            buffer[2*i+1] = lsettings->data[i];
        }

		if (client_list->NbClient<=client_list->Nmax)
		{
            // lines added to clear valgrind error: points to unaddressable byte(s)
            SOCKADDR_IN toto = {0};
            tmp = accept (sock_server, (SOCKADDR *)&toto, &socklen);
            client_list->sin_client[client_list->NbClient] = toto;//(SOCKADDR_IN)toto;
			//tmp=accept(sock_server,(SOCKADDR *)&client_list->sin_client[client_list->NbClient],&socklen);
			client_list->sock_client[client_list->NbClient]=tmp; //strangely we have to pass by a temporary variable SOCKET or their is a problem when transmetting a socket after we clear a client
			if (client_list->sock_client[client_list->NbClient]==INVALID_SOCKET)
			{
				perror("accept()");
				exit(errno);
			}
			else
			{
                printf("send settings via tcp\n");
				client_list->NbClient+=1;
				printf("Client number %i on %i, connected on socket = %i\n",client_list->NbClient,client_list->Nmax,client_list->sock_client[client_list->NbClient-1]);
                send_TCP_server(client_list, (char*)buffer, buffer_size, client_list->NbClient-1);
                printf("settings sent\n");
			}
		}	
		else{printf("too many clients\n");}
}

void clear_client(client* client_list, unsigned int id)
{
	int i;
	client client_temp={};
	init_struct_client(&client_temp,client_list->Nmax);
	if (client_list->NbClient>1)
	{
        //if the last has quit we simply copy to temporary client sturct except last client
		if (id==client_list->NbClient-1)
		{
			for (i=0 ; i<id ; i++)
			{
				client_temp.sock_client[i]=client_list->sock_client[i];
				client_temp.sin_client[i]=client_list->sin_client[i];
				client_temp.id_client[i]=client_list->id_client[i];
			}
		}
		else //else we copy 0->id-1 and id+1->NbClient 
		{
			for (i=0 ; i<id ; i++)
			{
				client_temp.sock_client[i]=client_list->sock_client[i];
				client_temp.sin_client[i]=client_list->sin_client[i];
				client_temp.id_client[i]=client_list->id_client[i];
			}
			for (i=id+1 ; i<client_list->NbClient ; i++)
			{
				client_temp.sock_client[i-1]=client_list->sock_client[i];
				client_temp.sin_client[i-1]=client_list->sin_client[i];
				client_temp.id_client[i-1]=client_list->id_client[i];
			}
		}
	}

	//recopy without loss temporary client struct in client_list
	for (i=0 ; i<client_list->NbClient-1 ; i++)
	{
		client_list->sock_client[i]=client_temp.sock_client[i];
		client_list->sin_client[i]=client_temp.sin_client[i];
	}
	client_list->NbClient-=1;
}


//Conversion function
int int_converter(char x)
{
	int y=0;
	y=(int)x+256;
	y=y%256;
	return y;
}


//TCP server function

int init_TCP_server(SOCKET* sock, int Port,client* client_list, unsigned int MaxClient)
{
	SOCKADDR_IN server={0};

	//Create socket
	(*sock)=socket(AF_INET, SOCK_STREAM,0);
	if ((*sock)==INVALID_SOCKET)
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
	if (setsockopt((*sock),SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) 
	{
		printf("error using setsockopt()");
		return 2;
	}

	//Link communication point
	if (bind((*sock), (SOCKADDR *)&server, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		printf("error using bind()");
		return 3;
	}

	//Connexion
	if (listen((*sock), MaxClient)==SOCKET_ERROR)
	{
		printf("error using listen()");
		return 4;
	}

	init_struct_client(client_list, MaxClient);
    return 0;
}

int init_TCP_server_simple(SOCKET *server_socket, SOCKET *client_socket, SOCKADDR_IN *client_sin, int Port)
{
	SOCKADDR_IN server = {0};
    unsigned int socklen=sizeof(SOCKADDR_IN);
    //SOCKET client_socket=0;

	//Create socket
	(*server_socket)=socket(AF_INET, SOCK_STREAM,0);
	if ((*server_socket)==INVALID_SOCKET)
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
	if (setsockopt((*server_socket),SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(int)) == -1) 
	{
		printf("error using setsockopt()");
		return 2;
	}

	//Link communication point
	if (bind((*server_socket), (SOCKADDR *)&server, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		printf("error using bind()");
		return 3;
	}

	//Connexion
	if (listen((*server_socket), 1)==SOCKET_ERROR)
	{
		printf("error using listen()");
		return 4;
	}

    //wait for client
	(*client_socket) = accept( (*server_socket), (SOCKADDR *)client_sin, &socklen);
    if ( (*client_socket) == INVALID_SOCKET ) 
    {
        perror("accept()");
        exit(errno);
    }

    return 0;
}

void close_TCP_server(SOCKET* sock, client* client_list)
{
	int i=0;

	//pthread_cancel(TCP_server_thread);//close thread if used

	//for(i=0 ; i<client_list->NbClient ; i++){close(client_list->sock_client[i]);}
	for(i=0 ; i<client_list->NbClient ; i++)
    {
        if (shutdown(client_list->sock_client[i],SHUT_RDWR)!=0)
		{
			perror("shutdown server");
			exit(1);
		}
        if (close(client_list->sock_client[i])!=0)
		{
			perror("close");
			exit(1);
		}
    }

	close((*sock));
	clear_struct_client(client_list);
}

void close_TCP_server_simple(SOCKET* sock, SOCKET *client)
{
    if (shutdown((*client),SHUT_RDWR)!=0)
    {	
		perror("shutdown server");
		exit(1);
	}
    if (close((*client))!=0)
	{
		perror("close");
		exit(1);
	}

	close((*sock));
}

int send_TCP_server(client* client_list, char* buffer, int buff_length, int target)
{
	int i=0;
	int err=0;

	//local variable to clear the client after sending, whereas due to reorganisation it bug if we don't clear the last client
	int tmp[client_list->Nmax+1];
	for (i=0 ; i<client_list->Nmax+1 ; i++){tmp[i]=0;}

    printf("send_TCP_server function, target = %i\n",target);
	if (target<0)
	{
		for (i=0 ; i<client_list->NbClient ; i++)
		{
			if(send(client_list->sock_client[i], buffer, buff_length, MSG_NOSIGNAL)<0)
			{
				printf("Client %i disconnected\n",client_list->id_client[i]+1);
				//clear_client(client_list, client_list->id_client[i]);
				tmp[0]+=1;
				tmp[tmp[0]]=client_list->id_client[i];
				err=1;
			}
		}

		//Clearing disconnected clients
		for (i=tmp[0] ; i>0 ; i--)
		{
			clear_client(client_list, tmp[i]);
		}
	}
	else
	{
        //valgrind error in send, points to uninitialised byte(s)
        //error disappear only if I reaffect value to all the buffer...
		if(send(client_list->sock_client[target], buffer, buff_length, MSG_NOSIGNAL)<0)
		{
			printf("Client %i disconnected\n",client_list->id_client[target]+1);
			clear_client(client_list, client_list->id_client[target]);
			err=1;
		}
	}

	return err;
}

int receive_TCP_server(client* client_list, char* buffer, int buff_length, int target)
{
    /*
	if (recv(client_list->sock_client[target], buffer, buff_length+1, MSG_WAITALL)==0)
	{
		printf("client disconnected\n");
		return 1;
	}
	else {return 0;}
*/
    int tcp_size=0;
    tcp_size = recv(client_list->sock_client[target], buffer, buff_length+1, MSG_PEEK);
    printf("tcp size = %i\n", tcp_size);
	if (tcp_size <= 0)
	{
		printf("client disconnected\n");
		return 1;
	}
	else 
    {
        recv(client_list->sock_client[target], buffer, tcp_size, MSG_WAITALL);
        return 0;
    }

}

int send_int16_TCP_server(client* client_list, int16_t *buffer, int buff_length, int target)
{
	int i=0;
	int err=0;

	//local variable to clear the client after sending, whereas due to reorganisation it bug if we don't clear the last client
	int tmp[client_list->Nmax+1];
	for (i=0 ; i<client_list->Nmax+1 ; i++){tmp[i]=0;}

	if (target<0)
	{
		for (i=0 ; i<client_list->NbClient ; i++)
		{
			if(send(client_list->sock_client[i], (char *)buffer,2*buff_length, MSG_NOSIGNAL)<0)
			{
				printf("Client %i disconnected\n",client_list->id_client[i]+1);
				//clear_client(client_list, client_list->id_client[i]);
				tmp[0]+=1;
				tmp[tmp[0]]=client_list->id_client[i];
				err=1;
			}
		}

		//Clearing disconnected clients
		for (i=tmp[0] ; i>0 ; i--)
		{
			clear_client(client_list, tmp[i]);
		}
	}
	else
	{
		if(send(client_list->sock_client[target], (char *)buffer, 2*buff_length, MSG_NOSIGNAL)<0)
		{
			printf("Client %i disconnected\n",client_list->id_client[target]+1);
			clear_client(client_list, client_list->id_client[target]);
			err=1;
		}
	}

	return err;
}

int receive_int16_TCP_server(client* client_list, uint16_t* buffer, int buff_length, int target)
{
	if (recv(client_list->sock_client[target], (char *)buffer, 2*buff_length, MSG_WAITALL)==0)
	{
		printf("client disconnected\n");
        clear_client(client_list, client_list->id_client[target]);
		return 1;
	}
	else {return 0;}
}

//Possible thread for managing server
/*
void *TCP_server_routine(void* p_data)
{
	server_info* serv_info=(server_info*)p_data;
	
	while (1)
	{
		add_client(serv_info->client_list, serv_info->RP_settings,serv_info->sock);
		printf("client number %i connected\n",serv_info->client_list->NbClient);
	}

	return NULL;
}

void launch_server(SOCKET* sock, client* client_list, Settings *nSettings)
{
	server_info* serv_info=(server_info *)malloc(sizeof(server_info));
	serv_info->sock = (*sock);
	serv_info->client_list = client_list;
    serv_info->RP_settings = nSettings;

	if(pthread_create(&TCP_server_thread, NULL, TCP_server_routine, serv_info)!=0)
	{
		perror("pthread_create()");
		exit(errno);
	}
}

void *Settings_server_routine(void *p_data)
{
    printf("settings thread beggin\n");
    server_info* serv_info=(server_info*)p_data;
    uint8_t buffer[2*settings_size];
    while(serv_info->client_list->NbClient>0)
    {
        printf("in while loop of settings thread\n");
        receive_TCP_server(serv_info->client_list, (char *)buffer, sizeof(buffer), 0);
        Settings_edit_from_TCP_buffer(serv_info->RP_settings, buffer);
        Settings_print(*serv_info->RP_settings);
        send_TCP_server(serv_info->client_list, (char *)buffer, sizeof(buffer), 0);
    }
    printf("killing settings thread\n");
    pthread_exit(NULL);
}

void launch_settings_comm(SOCKET *sock, client* client_list, Settings *nSettings)
{
	server_info* serv_info=(server_info *)malloc(sizeof(server_info));
	serv_info->sock = (*sock);
	serv_info->client_list = client_list;
    serv_info->RP_settings = nSettings;

    if(pthread_create(&Settings_server_routine_thread, NULL, Settings_server_routine, serv_info) != 0)
    {
        perror("pthread_create()");
        exit(errno);
    }
}
*/

//TCP client function
int init_TCP_client(SOCKET* sock, const char* IP, int Port)
{
	SOCKADDR_IN sclient={0};
	
	//Create socket
	(*sock)=socket(AF_INET, SOCK_STREAM, 0);
	if ((*sock)==INVALID_SOCKET)
	{
		printf("error using socket()");
		return 1;
	}

	//Socket info
	sclient.sin_addr.s_addr=inet_addr(IP);
	sclient.sin_family=AF_INET;
	sclient.sin_port=htons(Port);

	//Connection to server
	if (connect((*sock), (SOCKADDR *) &sclient, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		printf("error using connect()");
		return 2;
	}
	
	printf("Connected\n");
    return 0;
}

void close_TCP_client(SOCKET* sock)
{
	if (shutdown((*sock), SHUT_RDWR)!=0)
	{
		perror("shutdown client");
		exit(1);
	}
	if (close((*sock))!=0)
	{
		perror("close");
		exit(1);
	}
}

void send_TCP_client(SOCKET* sock, char* buffer, int buff_length)
{
	if(send((*sock), buffer, buff_length+1, MSG_WAITALL)<0)
	{
		perror("send()");
		exit(errno);
	}
}

int receive_TCP_client(SOCKET* sock, char* buffer, int buff_length)
{
	if (recv((*sock), buffer, buff_length, MSG_WAITALL)==0)
	{
		printf("server closed\n");
		return 1;
	}
	else {return 0;}
}

int peek_TCP_client(SOCKET* sock, int data_size)
{
    int size=0;
    char data[4];
    int32_t *tata;
    switch (data_size)
    {
        case 32:
            if (recv((*sock), data, data_size/8, MSG_PEEK)==0) {printf("server closed\n");}
            tata=(int32_t *)data;
            size=(int)(tata[0]);
            size=ntohl(size)*4;
            break;
        default:
            printf("peek_TCP_client not implemented yet for %i bits longs data", data_size);
            size=0;
            break;
    }
    return size;
}

void send_int16_TCP_client(SOCKET* sock, int16_t *buffer, int buff_length)
{
	if(send((*sock), (char *)buffer, 2*buff_length, 0)<0)
	{
		perror("send()");
		exit(errno);
	}
}

int receive_int16_TCP_client(SOCKET* sock, int16_t *buffer, int buff_length)
{
	if (recv((*sock), (char *)buffer, 2*buff_length, MSG_WAITALL)==0)
	{
		printf("server closed\n");
		return 1;
	}
	else {return 0;}
}










