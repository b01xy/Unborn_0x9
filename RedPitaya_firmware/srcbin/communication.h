#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include<pthread.h>
#include"settings.h"
#include"TCP_API.h"
#include"UDP_API.h"

pthread_t TCP_server_thread;
void *TCP_server_routine(void* p_data); //server routine function for thread, server turn in parallel to main
//void launch_server(SOCKET* sock, client* client_list, Settings *nSettings); //function that launch the server in parallel to main
void launch_server(server_info* serv_info);
pthread_t Settings_server_routine_thread;
void *Settings_server_routine(void *p_data);
//void launch_settings_comm(SOCKET *sock, client* client_list, Settings *nSettings);
void launch_settings_comm(server_info* serv_info);
void kill_thread();

pthread_t data_thread;
void *emulator_thread(void *p_data);
void simple_emulator(Settings *nSettings, SOCKADDR_IN tcp_client_sin);

#endif
