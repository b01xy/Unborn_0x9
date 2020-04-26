#ifndef SETTINGS_H
#define SETTINGS_H

#include<stdio.h>
#include<stdint.h>

typedef enum Settings_ID Settings_ID;
typedef struct Settings Settings;

void Settings_print(Settings lSettings);
void Settings_init(Settings *lSettings);
void Settings_edit(Settings *lSettings, Settings_ID ID, int value);
void Settings_edit_number_of_point(Settings *lSettings, int value);
void Settings_edit_delay(Settings *lSettings, int value);
int Settings_get_value(Settings lSettings, Settings_ID ID);
int Settings_get_number_of_point(Settings lSettings);
int Settings_get_delay(Settings lSettings);
void Settings_edit_from_TCP_buffer(Settings *lSettings, uint8_t *buff);

enum Settings_ID
{
    na = 0,
    tcp_udp,
    number_of_bit,
    decimation,
    number_of_line,
    number_of_point_msb,
    number_of_point_lsb,
    delay_msb,
    delay_lsb,
    angle,
    tgc_start,
    tgc_end,
    emulator,
    start,
    settings_size, //use to know automaticly number of settings
    client_id,
    ready, //use for RedPitaya 
    end
    //na
};

struct Settings
{
    uint8_t data[end+1];
};

#endif
