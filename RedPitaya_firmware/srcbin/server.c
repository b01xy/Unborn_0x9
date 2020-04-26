#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include"settings.h"
#include"TCP_API.h"
#include"communication.h"

int main(int argc, char *argv[])
{
    client lclient = {0,0,NULL,NULL,NULL};
    client *client_list = &lclient;
    Settings *RP_settings = (Settings *)malloc(sizeof(Settings));
    SOCKET TCP_SOCKET;
    init_TCP_server(&TCP_SOCKET, 7538, client_list, 1);
    Settings_init(RP_settings);
    Settings_print(*RP_settings);
   

    server_info* serv_info=(server_info *)malloc(sizeof(server_info));
    serv_info->sock = TCP_SOCKET;
    serv_info->client_list = client_list;
    serv_info->RP_settings = RP_settings;


    launch_server(serv_info);
    //launch_server(&TCP_SOCKET, client_list, RP_settings);
    
    while (RP_settings->data[end] != 1)
    {
        usleep(1000000);
        printf("waiting\n");
    }
    usleep(1100000);
    kill_thread();
   
    close_TCP_server(&TCP_SOCKET, client_list);
    clear_struct_client(client_list);
    //free(client_list);
    free(RP_settings);
    free(serv_info);

    return 0;
}
