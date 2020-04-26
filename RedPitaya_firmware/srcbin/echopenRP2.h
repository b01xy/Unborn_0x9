#ifndef ECHOPENRP_H
#define ECHOPENRP_H

#include<signal.h>

#include"rp.h"

void init_ramp(Settings lsettings);
void ramp(); //activate the ramp, the ramp is sent when a trigger event appears
void end_ramp(); //desactivate the ramp

void init_acquisition(Settings lsettings);
void trigg(Settings lsettings);
void on_trigger_acquisition(float *buffer_float, uint32_t buffer_length);

void init_RP(); //initialise RedPitaya
void close_RP(); //close RedPitaya
void init_pulse(); //initialise pulse GPIO (gpio used to trigger arduino nano)
void pulse(); //send pulse to trigger arduino nano

#endif
