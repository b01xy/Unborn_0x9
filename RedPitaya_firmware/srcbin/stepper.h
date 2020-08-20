#ifndef STEPPER_H
#define STEPPER_H

#include<unistd.h>

//#define StepNumber 400 //number of step per tour of the stepper motor
#define StepNumber 1024  //number of step for byj motor

typedef struct stepper_motor stepper_motor; //so we can call stepper_motor and not struct stepper_motor
/*---
enum mode is used for defining the steps of the stepper motor:
-full for full step
-full_2 for half step
-full_4 for 1/4 step
-full_8 for 1/8 step
-full_16 for 1/16 step
---*/
//TODO do we keep microstep option?
typedef enum mode mode;
/*---
enum sens is for the rotary sens of the stepper motor
---*/
typedef enum sens sens;

//functions to adapte depending on the hardware
void init_gpio(int pin, int value); //set gpio as input (value=0) or output (value=1)
void set_gpio(int pin, int value); //set value high (value=1) or low (value=0) to gpio pin
void wait(int time); //wait time in us

void init_stepper(stepper_motor* stepper); //initialyse the structure stepper_motor
void enable_stepper(stepper_motor* stepper);
void disable_stepper(stepper_motor* stepper);
void set_mode(stepper_motor* stepper, mode step_size); //set mode of the stepper : full, 1/2, 1/4, 1/8, 1/16 step
int half_step_time(stepper_motor* stepper, float* speed); //give time of a half step depending on the mode and speed in tour per second, note that this time is in microseconds and must be greater than 1 us. It change the speed to its real value
int step_number(stepper_motor* stepper, float* angle); //give the number of step to do in order to move to a given angle, if angle is not a multiple of the step angle, angle is change to its correct value
void move(stepper_motor* stepper, float* angle, float* speed, sens dir); //move the stepper of a given angle with a given speed on sens dir. It returns the real mooved angle and speed
void init_position(stepper_motor* stepper, float angle); //initialize the stepper position by going to the mecanic stop (sens2) and then move to the given angle (sens1)

enum mode
{
	full,full_2,full_4,full_8,full_16
};

enum sens
{
	sens1,sens2
};

struct stepper_motor
{
	int pin_en; //gpio for enabling or disabling stepper (via the stepper driver A4988, pin EN)
	int pin_ms1; //gpio for state of MS1 of A4988
	int pin_ms2; //gpio for state of MS2 of A4988
	int pin_ms3; //gpio for state of MS3 of A4988
	int pin_step; //gpio for pin STEP of A4988
	int pin_dir; //gpio for pin DIR of A4988
	mode step_size;
	int StepPerTour; //number of step per tour depending on the step size (full, 1/2, 1/4...)
	float minimum_angle; //angle made by one step size
};


#endif
