#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include"settings.h"
#include"TCP_API.h"
#include"UDP_API.h"

int main(int argc, char *argv[])
{
    SOCKET sock_client;
    const char *IP="0.0.0.0";
    init_TCP_client(&sock_client, IP, 7538);

    int tcp_size=2*settings_size;
    uint8_t buff[tcp_size];
    Settings client_settings;
    for (int i=0 ; i<2*settings_size ; i++) {buff[i]=0;}

    printf("waiting for tcp packet\n");
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    printf("settings received\n");
    for (int i=0 ; i<settings_size*2 ; i++)
    {
        printf("ID loop: %i, settings: %i\n", i, (buff[i]));
    }
    Settings_edit_from_TCP_buffer(&client_settings, buff);
    Settings_print(client_settings);

    //trying to edit settings with all the settings
    Settings_edit_number_of_point(&client_settings, 489);
    Settings_ID lID = na;
    for (lID=0 ; lID<settings_size ; lID++)
    {
        buff[2*lID] = lID;
        buff[2*lID+1] = client_settings.data[lID];
    }

    send_TCP_client(&sock_client, (char *)buff, tcp_size);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);

    Settings_edit_from_TCP_buffer(&client_settings, buff);
    Settings_print(client_settings);

    //edit less settings and launch emulator;
    //prepare settings
    uint8_t biff[6];
    for (int i=0 ; i<6 ; i++) {biff[i]=0;}
    biff[0] = start;
    biff[1] = 1;
    biff[2] = tcp_udp;
    biff[3] = 1;
    biff[4] = emulator;
    biff[5] = 1;
/*
    //init UDP server
    SOCKET d_sock=0;
    SOCKADDR_IN d_sockin={0};
    int sin_len = sizeof(d_sockin);
    init_UDP_server_no_validation(&d_sock, &d_sockin, 7539);
    int nb_point = Settings_get_number_of_point(client_settings);
    int16_t data[nb_point];
    for (int i=0 ; i<nb_point ; i++) {data[i] = 0;}
*/

    //init TCP server
    SOCKET server_socket = 0;
    SOCKET client_socket = 0;
    SOCKADDR_IN client_sin = {0};
    int nb_point = Settings_get_number_of_point(client_settings);
    int16_t data[nb_point];
    for (int i=0 ; i<nb_point ; i++) {data[i] = 0;}

    send_TCP_client(&sock_client, (char *)biff, 6);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    Settings_edit_from_TCP_buffer(&client_settings, buff);
    Settings_print(client_settings);

    //tcp server must be launch after asking start from RP, because it's a blocking function
    init_TCP_server_simple(&server_socket, &client_socket, &client_sin, 7539);
    recv(client_socket, (char *)data, 2*(nb_point+1), MSG_WAITALL);
    for (int i=0 ; i<nb_point ; i++) {printf("data[%i] = %i, ",i,data[i]);}
    printf("\n");
    close_TCP_server_simple(&server_socket, &client_socket);

    //stop emulator
    biff[0] = start;
    biff[1] = 0;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    usleep(2500000);

    //change number of bit
    for (int i=0 ; i<6 ; i++) {biff[i] = 0;}
    biff[0] = number_of_bit;
    biff[1] = 8;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);

    //launch emulator
    for (int i=0 ; i<6 ; i++) {biff[i] = 0;}
    biff[0] = start;
    biff[1] = 1;
    biff[2] = tcp_udp;
    biff[3] = 1;
    send_TCP_client(&sock_client, (char *)biff, 4);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);

    //lecture
    int8_t data_8b[nb_point];
    for (int i=0 ; i<nb_point ; i++) {data_8b[i] = 0;}
    init_TCP_server_simple(&server_socket, &client_socket, &client_sin, 7539);
    recv(client_socket, (char *)data_8b, (nb_point+1), MSG_WAITALL);
    for (int i=0 ; i<nb_point ; i++) {printf("data[%i] = %i, ",i,data_8b[i]);}
    printf("\n");
    usleep(2000000);


    //stop emulator
    for (int i=0 ; i<6 ; i++) {biff[i] = 0;}
    biff[0] = start;
    biff[1] = 0;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);

    //stop communication, close RP
    for (int i=0 ; i<6 ; i++) {biff[i] = 0;}
    biff[0] = end;
    biff[1] = 1;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    
    close_TCP_server_simple(&server_socket, &client_socket);
    
    return 0;
}
