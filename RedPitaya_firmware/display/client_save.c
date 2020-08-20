#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

#include"gnuplot_i.h"
#include"../srcbin/settings.h"
#include"../srcbin/TCP_API.h"
#include"../srcbin/UDP_API.h"

int main(int argc, char *argv[])
{
    SOCKET sock_client;
    const char *IP="192.168.128.3";//"0.0.0.0";
    init_TCP_client(&sock_client, IP, 7538);

    int tcp_size=2*settings_size;
    uint8_t buff[tcp_size];
    Settings client_settings;
    for (int i=0 ; i<2*settings_size ; i++) {buff[i]=0;}

    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    for (int i=0 ; i<settings_size*2 ; i++)
    {
        printf("ID loop: %i, settings: %i\n", i, (buff[i]));
    }
    Settings_edit_from_TCP_buffer(&client_settings, buff);
    Settings_print(client_settings);

    int TCP = 1;
    int bitsize = 12;
    int npoint = 2048*8;
    int dec = 1;

    //trying to edit settings with all the settings
    Settings_edit_number_of_point(&client_settings, npoint);
    Settings_edit_delay(&client_settings, 500);
    Settings_edit(&client_settings, decimation, dec);
    Settings_edit(&client_settings, number_of_bit, bitsize);
    Settings_edit(&client_settings, tgc_start, 55);
    Settings_edit(&client_settings, tgc_end, 155);
    Settings_edit(&client_settings, emulator, 0);
    Settings_edit(&client_settings, tcp_udp, TCP);
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
    uint8_t biff[2];
    for (int i=0 ; i<2 ; i++) {biff[i]=0;}
    biff[0] = start;
    biff[1] = 1;

    //udp server parameters
    SOCKET d_sock=0;
    SOCKADDR_IN d_sockin={0};
    int sin_len = sizeof(d_sockin);
    //TCP server parameters
    SOCKET server_socket = 0;
    SOCKET client_socket = 0;
    SOCKADDR_IN client_sin = {0};
    if (TCP == 0)
    {
        //init UDP server
        init_UDP_server_no_validation(&d_sock, &d_sockin, 7539);
    }


    int nb_point = Settings_get_number_of_point(client_settings);
    int16_t data[nb_point+1];
    int8_t data8[nb_point+1];
    int nline = (int)(client_settings.data[number_of_line]);
    int16_t image[nb_point+1][nline];
    for (int i=0 ; i<=nb_point ; i++) 
    {
        data[i] = 0;
	    data8[i] = 0;
    }
   
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    Settings_edit_from_TCP_buffer(&client_settings, buff);
    Settings_print(client_settings);
    
    if (TCP==1) //must send start befor launching server cause it's blocking till connection wich arrive with start
    {
        //init TCP server
        printf("init tcp server\n");
	    init_TCP_server_simple(&server_socket, &client_socket, &client_sin, 7539);
        printf("init done\n");
    }

    //init gnuplot
    gnuplot_ctrl *h;
    h = gnuplot_init();
    gnuplot_setstyle(h,"lines");
    gnuplot_cmd(h,"set xrange [0:%d]", npoint-1);
    //need to define double vector?
    double data_d[nb_point];

    for (int i=1 ; i<10*nline ; i++)
    {
        printf("loop number %i\n",i);
        if (bitsize ==8)
        {
			if (TCP == 0) {recvfrom(d_sock, (char *)data8, nb_point+1, 0, (SOCKADDR *)(&d_sockin), &sin_len);}
			else {recv(client_socket, (char *)data8, (nb_point+1), MSG_WAITALL);;}
            printf("line number %i\n", data8[0]);
        }
        else
        {
            if (TCP ==0) {recvfrom(d_sock, (char *)data, 2*(nb_point+1), 0, (SOCKADDR *)(&d_sockin), &sin_len);}
			else {recv(client_socket, (char *)data, 2*(nb_point+1), MSG_WAITALL);}
            printf("line number %i\n", data[0]);
            image[0][data[0]] = data[0];
        }
        for (int j=0 ; j<npoint ; j++)
        {
            if (bitsize == 8)
            {
                data_d[j] = (double)(data8[j+1]);
            }
            else
            {
                data_d[j] = (double)(data[j+1]);
                image[j+1][data[0]] = data[j];
            }
        }
        gnuplot_resetplot(h);
        gnuplot_plot_x(h, data_d, npoint-1, "RP test");
        if ( (i%nline)==0 )
        {
            FILE *h = fopen("image.txt", "w+");
            for (int j=0 ; j<npoint+1 ; j++)
            {
                for (int k=0 ; k<nline ; k++)
                {
                    fprintf(h, "%i ", image[j][k]);
                }
                fprintf(h, "\n");
            }
            fclose(h);
        }
    }

    //stop emulator
    biff[0] = start;
    biff[1] = 0;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    usleep(2500000);


    //stop communication, close RP
    for (int i=0 ; i<2 ; i++) {biff[i] = 0;}
    biff[0] = end;
    biff[1] = 1;
    send_TCP_client(&sock_client, (char *)biff, 2);
    receive_TCP_client(&sock_client, (char *)buff, tcp_size);
    
    //close server
    if (TCP == 0)
    {
    	close_UDP_server(&sock_client);
	}
	else
	{
		close_TCP_server_simple(&server_socket, &client_socket);
	}
    
    return 0;
}
