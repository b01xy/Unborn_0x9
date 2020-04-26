#include"settings.h"

void Settings_print(Settings lSettings)
{
    printf("\nRedPitaya settings:\n");
    float true_sampling_rate = 0.f, sampling_time = 0.f;
    true_sampling_rate = 125.f/((float)lSettings.data[decimation]);
    sampling_time = 1.f/true_sampling_rate;
    float l0 = 0.f, lf = 0.f;
    l0 = ((float)lSettings.data[tgc_start]) / 255.f;
    lf = ((float)lSettings.data[tgc_end]) / 255.f;

    if (lSettings.data[tcp_udp] == 0)
    {
        printf("Data send via UDP\n");
    }
    else
    {
        printf("Data send via TCP\n");
    }
    printf("Data encoded in %i bits\n", lSettings.data[number_of_bit]);
    printf("Decimation: %i => true sampling rate: %f\n", lSettings.data[decimation], true_sampling_rate);
    printf("Number of line per image: %i\n", lSettings.data[number_of_line]);
    printf("Number of point per line: %i\n", Settings_get_number_of_point(lSettings));
    printf("Delay: %i point => %f us, or %f mm (c=1480m.s^{-1}\n)", Settings_get_delay(lSettings), sampling_time*((float)Settings_get_delay(lSettings)), sampling_time*((float)Settings_get_delay(lSettings))*0.74f);
    printf("Angle of the sector of the image: %i\n", lSettings.data[angle]);
    printf("TGC value at beggining of measurement: %f V, => %f dB (LO mode), => %f dB (HI mode)\n", l0, -4.5f + 48.f*l0, 7.5f + 48.f*l0);
    printf("TGC value at end of measurement: %f V, => %f dB (LO mode), => %f dB (HI mode)\n", lf, -4.5f + 48.f*lf, 7.5f + 48.f*lf);
    if (lSettings.data[emulator] == 0)
    {
        printf("RedPitaya send measured data\n");
    }
    else
    {
        printf("RedPitaya send emulated data\n");
    }
    if (lSettings.data[start] == 1)
    {
        printf("RedPitaya is running\n");
    }
    else
    {
        printf("RedPitaya is waiting start command\n");
    }
}

void Settings_init(Settings *lSettings)
{
    lSettings->data[tcp_udp] = 1; //0->UDP, 1->TCP
    lSettings->data[number_of_bit] = 16; //8 or 16
    lSettings->data[decimation] = 1; //1 or 8 or 16...
    lSettings->data[number_of_line] = 64; //less than 128
    Settings_edit_number_of_point(lSettings, 1024); //less than 65536
    Settings_edit_delay(lSettings, 0); //delay as number of point, max 32768
    lSettings->data[angle] = 60; //max 180
    lSettings->data[tgc_start] = 50; //TGC at begin of measurement, tension = value / 255
    lSettings->data[tgc_end] = 50; //TGC at end of measurement
    lSettings->data[emulator] = 0; //0->true measurement, 1->emulator
    lSettings->data[start] = 0; //begin measurement when receive start=1, stop when receive start=0
    lSettings->data[ready] = 0; //basic state not ready cause RP no set with this data
    lSettings->data[end] = 0; //may be used to kill all thread and stop the code
    lSettings->data[client_id] = 0; //use to know the IP of the client corresponding to this id
}

void Settings_edit ( Settings *lSettings, Settings_ID ID, int value )
{
    switch (ID)
    {
        case tcp_udp:
        case emulator:
        case start:
        case end:
            if (value == 0) {lSettings->data[ID] = 0;}
            else {lSettings->data[ID] = 1;}
            break;
        case number_of_bit:
            if (value <= 8) {lSettings->data[ID] = 8;}
            else {lSettings->data[ID] = 16;}
            break;
        case decimation:
            if (value<8) {value=1;}
            else {value = 8 * ( ((uint8_t)value)/8 );}
            lSettings->data[ID] = value;
            break;
        case number_of_line:
            if (value <= 10) {value = 10;} //we set a minimum of 10 lines
            printf("Warning we have to look at the minimum angle step to change sector or number of line\n");
            lSettings->data[ID] = (uint8_t)value;
            break;
        case angle:
            if (value <= 10) {value = 10;}
        case tgc_start:
        case tgc_end:
        case number_of_point_msb:
        case number_of_point_lsb:
        case delay_msb:
        case delay_lsb:
            if (value < 0) {value = 0;}
            else if (value > 255) {value = 255;}
            lSettings->data[ID] = (uint8_t)value;
            break;
        case na:
            break;
        default:
            printf("Warning trying to edit not editable (undefined ID=%i) settings\n", ID);
            break;
    }
    //lSettings->data[ID] = (uint8_t)(value);
}

void Settings_edit_number_of_point (Settings *lSettings, int value)
{
    if (value < 12)
    {
        printf("Warning, number of point to small, minimum is 12 actually\n");
        value = 12;
    }
    if (value > 16384)
    {
        printf("Warning, maximum number of point is 2^16-1 = 16384\n");
        value = 16384;
    }
    uint16_t np16 = value;
    uint8_t np8 = np16/256;
    lSettings->data[number_of_point_msb] = np8;
    lSettings->data[number_of_point_lsb] = np16-256*np8;
}

void Settings_edit_delay (Settings *lSettings, int value)
{
    if (value < 0)
    {
        printf("Warning, delay is negative\n");
        value = 0;
    }
    if (value > 16384)
    {
        printf("Warning, maximum number of point is 2^12 = 16384\n");
        value = 16384;
    }
    uint16_t np16 = value;
    uint8_t np8 = np16/256;
    lSettings->data[delay_msb] = np8;
    lSettings->data[delay_lsb] = np16-256*np8;
}

int Settings_get_value (Settings lSettings, Settings_ID ID)
{
    int value = (lSettings.data[ID] + 256)%256;
    return value;
}

int Settings_get_number_of_point(Settings lSettings)
{
    //Settings_ID ID = number_of_point_lsb;
    int value = Settings_get_value(lSettings, number_of_point_lsb);
    value += 256*Settings_get_value(lSettings, number_of_point_msb);
    return value;
}

int Settings_get_delay(Settings lSettings)
{
    int value = Settings_get_value(lSettings, delay_lsb);
    value += 256*Settings_get_value(lSettings, delay_msb);
    return value;
}

void Settings_edit_from_TCP_buffer(Settings *lSettings, uint8_t *buff)
{
    for (int i=0 ; i<settings_size ; i++)
    {
        Settings_edit(lSettings, buff[2*i], buff[2*i+1]);
        //printf("i = %i, buff[2i] = %i, buff[2i+1] = %i\n", i, buff[2*i], buff[2*i+1]);
        //lSettings->data[buff[2*i]] = buff[2*i+1];
    }
}
