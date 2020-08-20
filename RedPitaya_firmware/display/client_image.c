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
    int npoint = 16384;//2048+512;
    //si le nombre de point est trop faible (par exemple 512) on fait seulement une image...
    int dec = 1;
    //decimation 1 : 16384 => 150 mm
    //decimation 8 : 2048 => 150 mm

    //trying to edit settings with all the settings
    Settings_edit_number_of_point(&client_settings, npoint);
    Settings_edit_delay(&client_settings, 0);
    Settings_edit(&client_settings, decimation, dec);
    Settings_edit(&client_settings, number_of_bit, bitsize);
    Settings_edit(&client_settings, tgc_start, 5);
    Settings_edit(&client_settings, tgc_end, 195);
    Settings_edit(&client_settings, emulator, 0);
    Settings_edit(&client_settings, tcp_udp, TCP);
    Settings_edit(&client_settings, angle, 90);
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
    //int16_t image[nb_point+1][Settings.data[number_of_line]];
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
    gnuplot_cmd(h, "set pm3d map");
    gnuplot_cmd(h, "set palette gray");


    //gnuplot_setstyle(h,"lines");
    //gnuplot_cmd(h,"set xrange [0:%d]", npoint-1);
    //need to define double vector?
    double data_d[nb_point];
    int nline = Settings_get_value(client_settings, number_of_line);

    int im_point = 256;
    double **data_image = NULL;
    data_image = (double**)malloc(nline*sizeof(double*));
    for (int j=0 ; j<nline ; j++)
    {
        data_image[j] = (double*)malloc(im_point*sizeof(double));
    }
    int **data_save = NULL;
    data_save = (int **)malloc(nline*sizeof(int*));
    for (int j=0 ; j<nline ; j++)
    {
        data_save[j] = (int*)malloc(npoint*sizeof(int));
    }

    int nb_image = 0;
    int nzero = 0;
    int new_line = 0;
    int old_line = 0;
    int nm = nb_point/im_point;
    double tmp = 0.0;
    
    while (nb_image < 25)
    {
        if (bitsize ==8)
        {
			if (TCP == 0) {recvfrom(d_sock, (char *)data8, nb_point+1, 0, (SOCKADDR *)(&d_sockin), &sin_len);}
			else {recv(client_socket, (char *)data8, (nb_point+1), MSG_WAITALL);}
            new_line = (int)(data8[0]);
        }
        else
        {
            if (TCP ==0) {recvfrom(d_sock, (char *)data, 2*(nb_point+1), 0, (SOCKADDR *)(&d_sockin), &sin_len);}
			else {recv(client_socket, (char *)data, 2*(nb_point+1), MSG_WAITALL);}
            new_line = (int)(data[0]);
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
            }
        }
       
        //for (int k=0 ; k<npoint ; k++)
        for (int k=0 ; k<im_point ; k++)
        {
            //if (new_line == nb_image)
            //{
            //    printf("toto\n");
            //    data_image[new_line][k] = 0.0;
            //}
            //else
            //{
            tmp = 0.0;
            for (int l=0 ; l<nm ; l++)
            {
                tmp += abs(data_d[nm*k+l]);
                //data_image[new_line][k] = abs(data_d[k]);
            }
            data_image[new_line][k] = tmp;
            //}
        }
        for (int k=0 ; k<npoint ; k++)
        {
            data_save[new_line][k] = data_d[k];
        }
        if ( (old_line < nline-1) && (new_line == nline-1) )
        {
            printf("image number %i\n",nb_image);
            gnuplot_resetplot(h);
            gnuplot_matrixdouble(h, data_image, im_point, nline);
            nb_image++;
        }
        if ( (old_line > 0) && (new_line == 0) )
        {
            printf("image number %i\n",nb_image);
            gnuplot_resetplot(h);
            gnuplot_matrixdouble(h, data_image, im_point, nline);
            nb_image++;
        }
        //gnuplot_resetplot(h);
        //gnuplot_plot_x(h, data_d, npoint-1, "RP test");
        old_line = new_line;
    }

    FILE *hs = fopen("image_hand.txt", "w+");
    for (int j=0 ; j<npoint+1 ; j++)
    {
        for (int k=0 ; k<nline ; k++)
        {
            fprintf(hs, "%i ", data_save[k][j]);
        }
        fprintf(hs, "\n");
    }
    fclose(hs);

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

    for (int j=0 ; j<nline ; j++)
    {
        free(data_image[j]);
        free(data_save[j]);
    }
    free(data_image);
    free(data_save);
    
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
