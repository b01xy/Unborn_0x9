#include"settings.h"
#include"stepper.h"
#include"echopenRP2.h"

#include"communication.h"

void *TCP_server_routine(void* p_data)
{
	server_info* serv_info=(server_info*)p_data;
	//int trigger=0;

    printf("start tcp server thread\n");
	while (1)
    //while ((serv_info->client_list->NbClient<1) && (serv_info->RP_settings->data[end]!=1) )
	{
        printf("in while loop tcp server thread\n");
		add_client(serv_info->client_list, serv_info->RP_settings,serv_info->sock);
        if (serv_info->client_list->NbClient == 1)
        {
            launch_settings_comm(serv_info);
        }
		printf("client number %i connected\n",serv_info->client_list->NbClient);
	}

    printf("end of thread TCP server routine\n");
    pthread_exit(NULL);
}

//void launch_server(SOCKET* sock, client* client_list, Settings *nSettings)
void launch_server(server_info* serv_info)
{
	if(pthread_create(&TCP_server_thread, NULL, TCP_server_routine, serv_info)!=0)
	{
		perror("pthread_create()");
		exit(errno);
	}
    pthread_detach(TCP_server_thread);
}

void *Settings_server_routine(void *p_data)
{
    server_info* serv_info=(server_info*)p_data;
    uint8_t buffer[2*settings_size];
    int err = 0;
    while( (serv_info->client_list->NbClient > 0) && (serv_info->RP_settings->data[end] != 1) )
    {
        printf("reading settings loop start\n");
        for (int i=0 ; i<2*settings_size ; i++) {buffer[i]=0;}
        err = receive_TCP_server(serv_info->client_list, (char *)buffer, sizeof(buffer), 0);
        if(!err)
        {
            printf("received settings:\n");
            for (int i=0 ; i<2*settings_size ; i++)
            {
                printf("buff[%i] = %i, ", i, buffer[i]);
            }
            printf("\n");
            Settings_edit_from_TCP_buffer(serv_info->RP_settings, buffer);
            Settings_print(*serv_info->RP_settings);
            send_TCP_server(serv_info->client_list, (char *)buffer, sizeof(buffer), 0);

            if (serv_info->RP_settings->data[end] == 1)
            {
                if (serv_info->RP_settings->data[start] == 1)
                {
                    printf("red pitaya is running, killing data thread properly\n");
                    serv_info->RP_settings->data[start] = 0;
                    usleep(1000000);
                }
            }
            else if (serv_info->RP_settings->data[start] == 1) 
            {
                //TODO we have to see how we manage the ID of client in the RP_settings
                pthread_create(&data_thread, NULL, emulator_thread, serv_info);
                pthread_detach(data_thread);
            }
               //simple_emulator(serv_info->RP_settings);} //neeed to open a thread for this
            else {printf("I do no...\n");}
        }
        else {clear_client(serv_info->client_list, 1);}
    }
    printf("exit thread settings\n");
    pthread_exit(NULL);
}

void kill_thread()
{
    pthread_cancel(TCP_server_thread);
    pthread_cancel(Settings_server_routine_thread);
}

//void launch_settings_comm(SOCKET *sock, client* client_list, Settings *nSettings)
void launch_settings_comm(server_info *serv_info)
{
    if(pthread_create(&Settings_server_routine_thread, NULL, Settings_server_routine, serv_info) != 0)
    {
        perror("pthread_create()");
        exit(errno);
    }
    pthread_detach(Settings_server_routine_thread);
}

void *emulator_thread(void *p_data)
{
    server_info* serv_info=(server_info*)p_data;
    Settings *lset = serv_info->RP_settings;
    SOCKADDR_IN tcp_client_sin = serv_info->client_list->sin_client[0];

    simple_emulator(lset, tcp_client_sin);
    pthread_exit(NULL);
}

void simple_emulator(Settings *nSettings, SOCKADDR_IN tcp_client_sin)
{
    printf("Simple emulator is running! Bazinga\n");

    int npoint = 0;
    npoint = Settings_get_number_of_point(*nSettings);
    int8_t *line_pointer_8b=NULL;
    int16_t *line_pointer_16b=NULL;
    int **image=NULL;
    //int data_port = 7539;
    SOCKET server_data_socket = 0; //TODO must be clean latter, will not be used
    SOCKET client_data_socket = 0;
    SOCKADDR_IN client_data_sin = {0}; //TODO rename latter cause it's the server sin
    int PORT = 7539;

    client_data_sin.sin_addr.s_addr = tcp_client_sin.sin_addr.s_addr;
    char *IP = inet_ntoa(tcp_client_sin.sin_addr);

    //init redpitaya
    float *data_pointer = NULL;
    init_RP();
    init_pulse();
    init_acquisition(*nSettings);
    init_ramp(*nSettings);

    //init stepper
    float sector = (float)(Settings_get_value(*nSettings, angle));
    int nline = Settings_get_value(*nSettings, number_of_line);
    int dec = Settings_get_value(*nSettings, decimation);
    float speed = 3.f;
    stepper_motor mot;
    init_stepper(&mot);
    float angle = sector/((float)(nline-1));
    set_mode(&mot, full); //TODO remove mode, for byj motor, no micro step
    enable_stepper(&mot);
    init_position(&mot, 0.f);
    sens dir = sens1;

    //create container for data line
    if (nSettings->data[number_of_bit] == 8)
    {
        line_pointer_8b = (int8_t *)calloc( npoint+1, sizeof(int8_t));

        for(int i=0 ; i<npoint ; i++)
        {
            line_pointer_8b[i+1] = (i-128)%256;
        }
        printf("initialize 8 bits data: \n");
        //for (int i=0 ; i<npoint ; i++) {printf("line[%i] = %i, ", i, line_pointer_8b[i]);}
        //printf("\n");
    }
    else
    {
        line_pointer_16b = (int16_t *)calloc(npoint+1, sizeof(int16_t));

        for(int i=0 ; i<npoint ; i++)
        {
            line_pointer_16b[i+1] = (i-2048)%4096;
        }
        printf("initialize 16 bits data: \n");
    }
    if (nSettings->data[emulator] == 0)
    {
        printf("init float buffer\n");
        data_pointer = (float *)calloc( npoint, sizeof(float) );
    }
    else
    {
        image = (int**)calloc( nline, sizeof(int*) );
        for (int i=0 ; i<64 ; i++)
        {
            image[i] = (int*)calloc( 16384, sizeof(int) );
        }
        FILE *h;
        h = fopen("image_hand.txt", "r");
        if (h==NULL) {printf("can't open file...\n");}
        int err=0;
        int val = 0;
        for (int i=0 ; i<16384 ; i++)
        {
            for (int j=0 ; j<64 ; j++)
            {
                err = fscanf(h, "%i", &val);
                image[j][i] = val;
                if (!err) {printf("error loop i = %i, j = %i\n", i, j);}
                //printf("err = %i, val = %i\n", err, val);
            }
        }
        fclose(h);
    }
    
    //create server
    if (nSettings->data[tcp_udp] == 0)
    {
        //UDP client
        //we use UDP client because if we use server, the client must send a message to the server
        //to know it's socket, and we don't want to
        printf("init UDP client\n");
        init_UDP_client3(&client_data_socket, &client_data_sin, PORT);
    }
    else
    {
        //TCP client
        //for symetrie we also use TCP client
        init_TCP_client(&client_data_socket, IP, PORT);
    }
    printf("client initialized\n");

    int line = 0;
    int tline = 100000;
    
    while (nSettings->data[start] == 1)
    {
        //envoyer les données en fonction de la socket. Qu'est qu'on peut sortir dans des sous-fonction?
        //si deux client quand on demande un changement de paramétrage, on arrête la socket data ensuite on réinitialise on renvoie le nouveau paramétrage à tous les clients quand c'est fait on relance la mesure
       
        if (nSettings->data[tcp_udp] == 0)
        {
            if (nSettings->data[number_of_bit] == 8)
            {
                int tmp8b = 0;
                if (nSettings->data[emulator] == 0)
                {
                    trigg(*nSettings);
                    pulse();
                    on_trigger_acquisition( data_pointer,(uint32_t)npoint);
                    for (int i=0 ; i<npoint; i++)
                    {
                        tmp8b = 2048.f*data_pointer[i];
                        if (tmp8b < -128) {tmp8b = -128;}
                        else if (tmp8b > 127) {tmp8b = 127;}
                        line_pointer_8b[i+1] = (int8_t)(tmp8b);//(int8_t) (2048.f*data_pointer[i]); //conversion en 12 bits, puis limitation en 8 bits
                    }
                    pulse();
                }
                else
                {
                    if (dec == 1)
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = image[line%nline][i];
                            if (tmp8b < -128) {tmp8b = -128;}
                            else if (tmp8b > 127) {tmp8b = 127;}
                            line_pointer_8b[i+1] = (int8_t)(tmp8b);
                        }
                    }
                    else
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = 0;
                            for (int j=0 ; j<8 ; j++)
                            {
                                tmp8b += image[line%nline][(8*i+j)%16384];
                            }
                            tmp8b /=8;
                            if (tmp8b < -128) {tmp8b = -128;}
                            else if (tmp8b > 127) {tmp8b = 127;}
                            line_pointer_8b[i+1] = (int8_t)(tmp8b);
                        }
                    }                   
                    tline = (int)(angle/(360.f*speed)*1000000.f);
                    usleep(tline);
                }
                line_pointer_8b[0] = (int8_t)line;
                sendto( client_data_socket, (char*)line_pointer_8b, npoint+1, 0, (SOCKADDR *)(&client_data_sin), sizeof(client_data_sin)); // no error check, we don't care actually if buffers are lost
                printf("line %i, udp, 8 bits\n",line);
                if (dir == sens1)
                {
                    if (line < nline-1) 
                    {
                        line++;
                        if (nSettings->data[emulator] == 0)
                        {
                            move(&mot, &angle, &speed, dir);
                        }
                    }
                    else {dir = sens2;}
                }
                else
                {
                    if (line > 0) 
                    {
                        line--;
                        if (nSettings->data[emulator] == 0)
                        {
                            move(&mot, &angle, &speed, dir);
                        }
                    }
                    else {dir  = sens1;}
                }
            }
            else
            {
                if (nSettings->data[emulator] == 0)
                {
                    // reference loop
                    trigg(*nSettings);
                    pulse();
                    on_trigger_acquisition( data_pointer,(uint32_t)npoint);
                    for (int i=0 ; i<npoint; i++)
                    {
                        //printf("data_f[%i] = %f, ",i,data_pointer[i]);
                        line_pointer_16b[i+1] = (int16_t) (2048.f*data_pointer[i]); //conversion en 12 bits, puis limitation en 8 bits
                    }
                    //printf("\n");
                    pulse();
                }
                else
                {
                    int tmp8b = 0;
                    if (dec == 1)
                    {
                        printf("dec 1, 16 bit\n");
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = image[line%nline][i];
                            line_pointer_16b[i+1] = (int16_t)(tmp8b);
                        }
                    }
                    else
                    {
                        printf("dec 8, 16 bit\n");
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = 0;
                            for (int j=0 ; j<8 ; j++)
                            {
                                tmp8b += image[line%nline][(8*i+j)%16384];
                            }
                            tmp8b /=8;
                            line_pointer_16b[i+1] = (int16_t)(tmp8b);
                        }
                    } 
                    tline = (int)(angle/(360.f*speed)*1000000.f);
                    usleep(tline);
                }
                line_pointer_16b[0] = (int16_t)line;
                sendto( client_data_socket, (char*)line_pointer_16b, 2*(npoint+1), 0, (SOCKADDR *)(&client_data_sin), sizeof(client_data_sin)); // no error check, we don't care actually if buffers are lost
                printf("line %i, udp, 16 bits\n",line);
                if (dir == sens1)
                {
                    if (line < nline-1) 
                    {
                        line++;
                        move(&mot, &angle, &speed, dir);
                    }
                    else {dir = sens2;}
                }
                else
                {
                    if (line > 0) 
                    {
                        line--;
                        move(&mot, &angle, &speed, dir);
                    }
                    else {dir  = sens1;}
                }
            }
        }
        else
        {
            if (nSettings->data[number_of_bit] == 8)
            {
                int tmp8b = 0;
                if (nSettings->data[emulator] == 0)
                {
                    trigg(*nSettings);
                    pulse();
                    on_trigger_acquisition( data_pointer,(uint32_t)npoint);
                    for (int i=0 ; i<npoint; i++)
                    {
                        tmp8b = 2048.f*data_pointer[i];
                        if (tmp8b < -128) {tmp8b = -128;}
                        else if (tmp8b > 127) {tmp8b = 127;}
                        line_pointer_8b[i+1] = (int8_t)(tmp8b);//(int8_t) (2048.f*data_pointer[i]); //conversion en 12 bits, puis limitation en 8 bits
                    }
                    pulse();
                }
                else
                {
                    if (dec == 1)
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = image[line%nline][i];
                            if (tmp8b < -128) {tmp8b = -128;}
                            else if (tmp8b > 127) {tmp8b = 127;}
                            line_pointer_8b[i+1] = (int8_t)(tmp8b);
                        }
                    }
                    else
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = 0;
                            for (int j=0 ; j<8 ; j++)
                            {
                                tmp8b += image[line%nline][(8*i+j)%16384];
                            }
                            tmp8b /=8;
                            if (tmp8b < -128) {tmp8b = -128;}
                            else if (tmp8b > 127) {tmp8b = 127;}
                            line_pointer_8b[i+1] = (int8_t)(tmp8b);
                        }
                    } 
                    tline = (int)(angle/(360.f*speed)*1000000.f);
                    usleep(tline);
                }
                line_pointer_8b[0] = (int8_t)line;
                send(client_data_socket, (char *)line_pointer_8b, npoint+1, MSG_WAITALL);
                printf("line %i, tcp, 8 bits\n",line);
                if (dir == sens1)
                {
                    if (line < nline-1) 
                    {
                        line++;
                        if (nSettings->data[emulator] == 0)
                        {
                            move(&mot, &angle, &speed, dir);
                        }
                    }
                    else {dir = sens2;}
                }
                else
                {
                    if (line > 0) 
                    {
                        line--;
                        if (nSettings->data[emulator] == 0)
                        {
                            move(&mot, &angle, &speed, dir);
                        }
                    }
                    else {dir  = sens1;}
                }
            }
            else
            {
                if (nSettings->data[emulator] == 0)
                {
                    // reference loop
                    trigg(*nSettings);
                    pulse();
                    on_trigger_acquisition( data_pointer,(uint32_t)npoint);
                    for (int i=0 ; i<npoint; i++)
                    {
                        //printf("data_f[%i] = %f, ",i,data_pointer[i]);
                        line_pointer_16b[i+1] = (int16_t) (2048.f*data_pointer[i]); //conversion en 12 bits, puis limitation en 8 bits
                    }
                    //printf("\n");
                    pulse();
                }
                else
                {
                    int tmp8b=0;
                    if (dec == 1)
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = image[line%nline][i];
                            line_pointer_16b[i+1] = (int16_t)(tmp8b);
                        }
                    }
                    else
                    {
                        for (int i=0 ; i<npoint ; i++)
                        {
                            tmp8b = 0;
                            for (int j=0 ; j<8 ; j++)
                            {
                                tmp8b += image[line%nline][(8*i+j)%16384];
                            }
                            tmp8b /=8;
                            line_pointer_16b[i+1] = (int16_t)(tmp8b);
                        }
                    } 
                   
                    tline = (int)(angle/(360.f*speed)*1000000.f);
                    usleep(tline);
                }
                line_pointer_16b[0] = (int16_t)line;
                send(client_data_socket, (char *)line_pointer_16b, 2*(npoint+1), MSG_WAITALL);
                if (dir == sens1)
                {
                    if (line < nline-1) 
                    {
                        line++;
                        move(&mot, &angle, &speed, dir);
                    }
                    else {dir = sens2;}
                }
                else
                {
                    if (line > 0) 
                    {
                        line--;
                        move(&mot, &angle, &speed, dir);
                    }
                    else {dir  = sens1;}
                }
            }
        }
    }

    //close client
    if (nSettings->data[tcp_udp] == 0)
    {
        //close UDP client
        close_UDP_client(&client_data_socket);
        printf("close UDP client\n");
    }
    else
    {
        //close TCP server
        close_TCP_server_simple( &server_data_socket, &client_data_socket);
        printf("close TCP client\n");
    }

    //delete container data line
    if (nSettings->data[number_of_bit] == 8)
    {
        free(line_pointer_8b);
    }
    else
    {
        free(line_pointer_16b);
    }
    if (data_pointer != NULL)
    {
        free(data_pointer);
    }
    if (image != NULL)
    {
        for (int i=0 ; i<nline ; i++)
        {
            free(image[i]);
        }
        free(image);
    }
    disable_stepper(&mot);
    end_ramp();
    close_RP();
    printf("emulator thread off\n");
    pthread_exit(NULL);
}
