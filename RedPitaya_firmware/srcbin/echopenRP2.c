#include<stdlib.h>

#include"settings.h"
#include"stepper.h"
#include"echopenRP2.h"

void init_ramp(Settings lsettings) 
{
	int i=0;
	uint32_t length=16384;
	float tmp=0.f, rmax=2.f*0.3f; //rmax maximum depth of measuremen: 20 cm
	float *homeramp;
	homeramp=(float *)malloc(length*sizeof(float));

    float level0 = (float)(Settings_get_value(lsettings, tgc_start)) / 255.f;
    float levelf = (float)(Settings_get_value(lsettings, tgc_end)) / 255.f;
    int Npoint = Settings_get_number_of_point(lsettings);
    float r0 = (float)(Settings_get_delay(lsettings)) * (float)(Settings_get_value(lsettings, decimation)) * 1480.f / 125000000.f;
    float rf = r0 + (float)(Npoint) * (float)(Settings_get_value(lsettings, decimation)) * 1480.f / 125000000.f;

	if(rf>rmax){rf=rmax;}

	float frequency=1480.0/rmax; //factor 2 for back and forth, give the frequency of the ramp (1 devide by time duration of the ramp), by default the time of the ramp is the time corresponding to rmax depth of measurement
	float lsamp=rmax/((double)length); //"length" of sampling, distance in meter between 2 points
	int N0=(int)(r0/lsamp); //r0 position in the buffer
	int Nf=(int)(rf/lsamp); //rf position in the buffer
	float slope=(levelf-level0)/((double)Nf-(double)N0); //slope of the ramp
	float initlevel=level0-slope*((double)N0); //level at first point

	//definition of the ramp
	for (i=0 ; i<length ; i++)
	{
		if (i<=Nf)
		{
			tmp=slope*((double)i)+initlevel;
			if (tmp<0.f){tmp=0.f;}
			homeramp[i]=tmp;
		}
		else {homeramp[i]=level0;}
	}

	//RedPitaya declaration for sending ramp on channel 1 (OUT1)
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_ARBITRARY);
	rp_GenArbWaveform(RP_CH_1, homeramp, length);
	rp_GenMode(RP_CH_1, RP_GEN_MODE_BURST);
	rp_GenBurstCount(RP_CH_1, 1);
	rp_GenAmp(RP_CH_1, 1.0);
	rp_GenFreq(RP_CH_1, frequency);
    rp_GenTriggerSource(RP_CH_1,RP_TRIG_SRC_CHA_PE); //to trigger on external trigger must put RP_TRIG_SRC_CHA_PE not RP_TRIG_SRC_EXT_PE...
	rp_GenOutEnable(RP_CH_1);

	free(homeramp);
}

void ramp()
{
	rp_GenOutEnable(RP_CH_1);
}

void end_ramp() {rp_GenOutDisable(RP_CH_1);}

void init_acquisition(Settings lsettings) 
{
    int dec = Settings_get_value(lsettings, decimation);
	if (dec==1)
	{
		rp_AcqSetDecimation(RP_DEC_1);
	}
	else if (dec==8)
	{
		rp_AcqSetDecimation(RP_DEC_8);
		rp_AcqSetAveraging(1); //average point over decimation
	}
	else
	{
		rp_AcqSetDecimation(RP_DEC_64);
		rp_AcqSetAveraging(1); //average point over decimation
	}
	
	rp_AcqSetGain(RP_CH_1,RP_LOW);
}

void trigg(Settings lsettings) 
{
    int ldelay = Settings_get_delay(lsettings) + Settings_get_number_of_point(lsettings) - 8192;
	rp_AcqStart();
	rp_AcqSetTriggerSrc(RP_TRIG_SRC_EXT_PE);
	rp_AcqSetTriggerDelay(ldelay); 
}

void on_trigger_acquisition(float *buffer_float, uint32_t buffer_length)
{
	rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING;

	//waiting for trigger event, on our systeme time is around some tens of nanoseconds
	while(1)
    {
	    rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED)
        {
			break;
		}
	}
	rp_AcqGetLatestDataV(RP_CH_1, &buffer_length, buffer_float);
}

void init_RP()
{
	if(rp_Init() != RP_OK)
	{
	printf("Rp api init failed!\n");
	}
	init_pulse();
}

void close_RP()
{
	rp_Release();
}

void init_pulse()
{
	rp_DpinSetDirection(RP_DIO7_N,1);
	rp_DpinSetState(RP_DIO7_N,1);
}

void pulse()
{
	rp_pinState_t state=0;
	rp_DpinGetState(RP_DIO7_N, &state);
	rp_DpinSetState(RP_DIO7_N, !state);
}

