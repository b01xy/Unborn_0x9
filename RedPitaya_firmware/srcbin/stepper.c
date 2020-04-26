#include<unistd.h>

#include"stepper.h"
#include"rp.h"

void init_gpio(int pin, int value)
{
	rp_DpinSetDirection(pin, value);
}


void set_gpio(int pin, int value)
{
	rp_DpinSetState(pin,value);
}

void wait(int time)
{
	usleep((unsigned int)time);
}


void init_stepper(stepper_motor* stepper)
{
	stepper->pin_en=RP_DIO5_N;
	stepper->pin_ms1=RP_DIO4_N;
	stepper->pin_ms2=RP_DIO3_N;
	stepper->pin_ms3=RP_DIO2_N;
	stepper->pin_step=RP_DIO1_N;
	stepper->pin_dir=RP_DIO0_N;
	stepper->step_size=full;
    //stepper->pin_en = RP_DIO0_N;
    //stepper->pin_dir = RP_DIO5_N;

	init_gpio(stepper->pin_en,1);
	init_gpio(stepper->pin_ms1,1);
	init_gpio(stepper->pin_ms2,1);
	init_gpio(stepper->pin_ms3,1);
	init_gpio(stepper->pin_step,1);
	init_gpio(stepper->pin_dir,1);


	set_gpio(stepper->pin_en,0);
	set_gpio(stepper->pin_ms1,0);
	set_gpio(stepper->pin_ms2,0);
	set_gpio(stepper->pin_ms3,0);
	set_gpio(stepper->pin_step,0);
	set_gpio(stepper->pin_dir,0);
}

void enable_stepper(stepper_motor* stepper)
{
	set_gpio(stepper->pin_en,1);
}

void disable_stepper(stepper_motor* stepper)
{
	set_gpio(stepper->pin_en,0);
}

void set_mode(stepper_motor* stepper, enum mode step_size)
{
	// mode of step from A4988 datasheet
	stepper->step_size=step_size;

	if (step_size==full)
	{
		set_gpio(stepper->pin_ms1,0);
		set_gpio(stepper->pin_ms2,0);
		set_gpio(stepper->pin_ms3,0);
		stepper->StepPerTour=StepNumber;
		stepper->minimum_angle=360.f/((float)stepper->StepPerTour);
	}
	if (step_size==full_2)
	{
		set_gpio(stepper->pin_ms1,1);
		set_gpio(stepper->pin_ms2,0);
		set_gpio(stepper->pin_ms3,0);
		stepper->StepPerTour=StepNumber*2;
		stepper->minimum_angle=360.f/((float)stepper->StepPerTour);
	}
	if (step_size==full_4)
	{
		set_gpio(stepper->pin_ms1,0);
		set_gpio(stepper->pin_ms2,1);
		set_gpio(stepper->pin_ms3,0);
		stepper->StepPerTour=StepNumber*4;
		stepper->minimum_angle=360.f/((float)stepper->StepPerTour);
	}
	if (step_size==full_8)
	{
		set_gpio(stepper->pin_ms1,1);
		set_gpio(stepper->pin_ms2,1);
		set_gpio(stepper->pin_ms3,0);
		stepper->StepPerTour=StepNumber*8;
		stepper->minimum_angle=360.f/((float)stepper->StepPerTour);
	}
	if (step_size==full_16)
	{
		set_gpio(stepper->pin_ms1,1);
		set_gpio(stepper->pin_ms2,1);
		set_gpio(stepper->pin_ms3,1);
		stepper->StepPerTour=StepNumber*16;
		stepper->minimum_angle=360.f/((float)stepper->StepPerTour);
	}
}

int half_step_time(stepper_motor* stepper, float* speed)
{
	float time;

	time=1000000.f/((float)stepper->StepPerTour)/(*speed)/2.f; //time in us (factor 1000000) of an half step (factor 2)
	//if (time<1.f){time=1.f;} //minimum time is set 1.f us from A4988 datasheet
    if (time<450.f) {time=450.f;} //empiric minimum time between 2 step for byj motor
	(*speed)=500000.f/((float)stepper->StepPerTour)/time; //from time=1000000/(Nstep*speed*2)

    //printf("time: %f us, speed: %f tr/s\n", time, *speed);
	return (int)time;
}

int step_number(stepper_motor* stepper, float* angle)
{
	int  Nstep;

	if ((*angle) < stepper->minimum_angle){(*angle)=stepper->minimum_angle;}	
	Nstep=(int)((*angle)/stepper->minimum_angle);
	(*angle)=(float)(Nstep)*stepper->minimum_angle;

    //printf("number of step: %i\n", Nstep);
	return Nstep;
}

void move(stepper_motor* stepper, float* angle, float* speed, sens dir)
{
	int Nstep, half_time;
	int i;

	set_gpio(stepper->pin_dir,dir);
	half_time=half_step_time(stepper, speed);
	Nstep=step_number(stepper, angle);

	for (i=0 ; i<Nstep ; i++)
	{
		//set_gpio(stepper->pin_dir, dir);
        set_gpio(stepper->pin_step,1);
		wait(half_time);
		set_gpio(stepper->pin_step,0);
        //set_gpio(stepper->pin_dir,0);
		wait(half_time);
	}
    //set_gpio(stepper->pin_dir,0);
}

void init_position(stepper_motor* stepper, float angle)
{
	float tour=360.f, speed=3.f;

	enable_stepper(stepper);
	wait(100);
	move(stepper, &tour, &speed, sens2);
	wait(100);
	move(stepper, &angle, &speed, sens1);

}
